#include <capstone/capstone.h>
#include <cassert>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/register.h>

#define CSS_ASSERT(cond) assert(cond)

namespace cppstacksize {
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

  Register_File registers;

  ::cs_insn* instructions;
  U64 base_address = 0;
  U64 instruction_count = ::cs_disasm(handle, code.data(), code.size(),
                                      base_address, 0, &instructions);

  S64 rsp_adjustment = 0;
  Stack_Map map;
  for (::cs_insn& instruction : std::span(instructions, instruction_count)) {
    ::cs_detail* details = instruction.detail;

    switch (instruction.id) {
      case ::X86_INS_MOV: {
        CSS_ASSERT(details->x86.op_count == 2);
        ::cs_x86_op* src = &details->x86.operands[1];
        ::cs_x86_op* dest = &details->x86.operands[0];
        if (dest->type == ::X86_OP_REG) {
          registers.store(dest->reg, *src);
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
          Register_Value src_value = registers.load(*src);
          if (src_value.kind == Register_Value_Kind::literal) {
            S64 increment = src_value.literal;
            // TODO(strager): Checked addition/subtraction.
            if (add) {
              rsp_adjustment += increment;
            } else {
              rsp_adjustment -= increment;
            }
          }
        }
        break;
      }

      case ::X86_INS_POP: {
        CSS_ASSERT(details->x86.op_count == 1);
        ::cs_x86_op* src = &details->x86.operands[0];
        rsp_adjustment += src->size;
        break;
      }

      case ::X86_INS_PUSH: {
        CSS_ASSERT(details->x86.op_count == 1);
        ::cs_x86_op* src = &details->x86.operands[0];
        rsp_adjustment -= src->size;
        map.touches.push_back(Stack_Map_Touch{
            .offset = narrow_cast<U32>(instruction.address),
            .entry_rsp_relative_address = rsp_adjustment,
            .byte_count = src->size,
            .access_kind = Stack_Access_Kind::write,
        });
        break;
      }
    }

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
              .entry_rsp_relative_address = rsp_adjustment + operand->mem.disp,
              .byte_count = operand->size,
              .access_kind = operand->access == ::CS_AC_WRITE
                                 ? Stack_Access_Kind::write
                                 : Stack_Access_Kind::read,
          });
        }
      }
    }
  }
  ::cs_free(instructions, instruction_count);
  ::cs_close(&handle);
  return map;
}
}
