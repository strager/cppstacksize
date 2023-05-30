#include <capstone/capstone.h>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/register.h>

namespace cppstacksize {
namespace {
Stack_Access_Kind stack_access_kind_from_capstone(/*::cs_ac_type*/ U8 access);
}

template <class Out, class In>
Out narrow_cast(In value) {
  return static_cast<Out>(value);
}

Stack_Map analyze_x86_64_stack_map(std::span<const U8> code) {
  ::csh handle;
  if (::cs_open(::CS_ARCH_X86, ::CS_MODE_64, &handle) != ::CS_ERR_OK) {
    // TODO(strager): Log an error.
    return Stack_Map();
  }
  ::cs_option(handle, ::CS_OPT_DETAIL, ::CS_OPT_ON);

  ::cs_insn* instructions;
  U64 base_address = 0;
  U64 instruction_count = ::cs_disasm(handle, code.data(), code.size(),
                                      base_address, 0, &instructions);

  Stack_Map map;
  map.registers.values[Register_Name::rsp] =
      Register_Value::make_entry_rsp_relative(0);
  auto get_rsp_adjustment_from_value = [](const Register_Value& value) -> S64 {
    if (value.kind != Register_Value_Kind::entry_rsp_relative) {
      // TODO(strager)
      return 0xcccccccc;
    }
    return value.entry_rsp_relative_offset;
  };
  auto get_rsp_adjustment = [&map, &get_rsp_adjustment_from_value]() -> S64 {
    Register_Value& rsp_value = map.registers.values[Register_Name::rsp];
    return get_rsp_adjustment_from_value(rsp_value);
  };
  for (::cs_insn& instruction : std::span(instructions, instruction_count)) {
    ::cs_detail* details = instruction.detail;

    switch (instruction.id) {
      case ::X86_INS_MOV:
      case ::X86_INS_MOVABS: {
        CSS_ASSERT(details->x86.op_count == 2);
        ::cs_x86_op* src = &details->x86.operands[1];
        ::cs_x86_op* dest = &details->x86.operands[0];
        if (dest->type == ::X86_OP_REG) {
          map.registers.store(dest->reg, *src);
        }
        break;
      }

      case ::X86_INS_ADD:
      case ::X86_INS_SUB: {
        bool add = instruction.id == ::X86_INS_ADD;
        CSS_ASSERT(details->x86.op_count == 2);
        ::cs_x86_op* src = &details->x86.operands[1];
        ::cs_x86_op* dest = &details->x86.operands[0];
        if (dest->type == ::X86_OP_REG && dest->reg == ::X86_REG_RSP) {
          Register_Value src_value = map.registers.load(*src);
          if (src_value.kind == Register_Value_Kind::literal) {
            S64 increment = src_value.literal;
            // TODO(strager): Checked addition/subtraction.
            if (add) {
              map.registers.add(dest->reg, increment);
            } else {
              map.registers.add(dest->reg, -increment);
            }
          }
        }
        break;
      }

      case ::X86_INS_POP: {
        CSS_ASSERT(details->x86.op_count == 1);
        ::cs_x86_op* src = &details->x86.operands[0];
        map.touches.push_back(Stack_Map_Touch{
            .offset = narrow_cast<U32>(instruction.address),
            .entry_rsp_relative_address = get_rsp_adjustment(),
            .byte_count = src->size,
            .access_kind = Stack_Access_Kind::read_only,
        });
        map.registers.add(::X86_REG_RSP, src->size);
        break;
      }

      case ::X86_INS_RET:
        map.touches.push_back(Stack_Map_Touch{
            .offset = narrow_cast<U32>(instruction.address),
            .entry_rsp_relative_address = get_rsp_adjustment(),
            .byte_count = 8,
            .access_kind = Stack_Access_Kind::read_only,
        });
        map.registers.add(::X86_REG_RSP, 8);
        break;

      case ::X86_INS_PUSH: {
        CSS_ASSERT(details->x86.op_count == 1);
        ::cs_x86_op* src = &details->x86.operands[0];
        map.registers.add(::X86_REG_RSP, -src->size);
        map.touches.push_back(Stack_Map_Touch{
            .offset = narrow_cast<U32>(instruction.address),
            .entry_rsp_relative_address = get_rsp_adjustment(),
            .byte_count = src->size,
            .access_kind = Stack_Access_Kind::write_only,
        });
        break;
      }
    }

    switch (instruction.id) {
      case ::X86_INS_LEA: {
        // Examples:
        // lea src, %rsp
        // lea 0x30(%rsp), %eax
        CSS_ASSERT(details->x86.op_count == 2);
        ::cs_x86_op* src = &details->x86.operands[1];
        CSS_ASSERT(src->type == ::X86_OP_MEM);
        ::cs_x86_op* dest = &details->x86.operands[0];

        // TODO(strager): What if index is present?
        map.registers.store(dest->reg, map.registers.load(src->mem.base));
        map.registers.add(dest->reg, src->mem.disp);
        if (src->mem.base == ::X86_REG_RSP) {
          map.touches.push_back(Stack_Map_Touch{
              .offset = narrow_cast<U32>(instruction.address),
              .entry_rsp_relative_address =
                  get_rsp_adjustment_from_value(map.registers.load(dest->reg)),
              .byte_count = (U32)-1,
              .access_kind = Stack_Access_Kind::read_or_write,
          });
        }
        break;
      }

      default:
        if (details->x86.op_count == 2) {
          for (U8 operand_index = 0; operand_index < 2; ++operand_index) {
            ::cs_x86_op* operand = &details->x86.operands[operand_index];
            if (operand->type == ::X86_OP_MEM &&
                operand->mem.base == ::X86_REG_RSP) {
              // Examples:
              // mov other_operand, (%rsp)
              // mov (%rsp), other_operand
              // movzbl (%rsp), other_operand
              map.touches.push_back(Stack_Map_Touch{
                  .offset = narrow_cast<U32>(instruction.address),
                  .entry_rsp_relative_address =
                      get_rsp_adjustment() + operand->mem.disp,
                  .byte_count = operand->size,
                  .access_kind =
                      stack_access_kind_from_capstone(operand->access),
              });
            }
          }
        }
        break;
    }
  }
  ::cs_free(instructions, instruction_count);
  ::cs_close(&handle);
  return map;
}

namespace {
Stack_Access_Kind stack_access_kind_from_capstone(/*::cs_ac_type*/ U8 access) {
  if (access == (::CS_AC_READ | ::CS_AC_WRITE)) {
    return Stack_Access_Kind::read_and_write;
  } else if (access == ::CS_AC_WRITE) {
    return Stack_Access_Kind::write_only;
  } else {
    return Stack_Access_Kind::read_only;
  }
}
}
}
