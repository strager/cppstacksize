#include <capstone/capstone.h>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/register.h>
#include <optional>
#include <unordered_map>

namespace cppstacksize {
namespace {
Stack_Access_Kind stack_access_kind_from_capstone(/*::cs_ac_type*/ U8 access);
}

bool is_read(Stack_Access_Kind sak) {
  switch (sak) {
    case Stack_Access_Kind::read_and_write:
    case Stack_Access_Kind::read_only:
    case Stack_Access_Kind::read_or_write:
      return true;
    case Stack_Access_Kind::write_only:
      return false;
  }
}

bool is_write(Stack_Access_Kind sak) {
  switch (sak) {
    case Stack_Access_Kind::write_only:
    case Stack_Access_Kind::read_or_write:
    case Stack_Access_Kind::read_and_write:
      return true;
    case Stack_Access_Kind::read_only:
      return false;
  }
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
      Register_Value::make_entry_rsp_relative(0, 0);
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

  // Byte offset of the most recently-encountered call instruction.
  U32 last_call_offset = 0;

  for (::cs_insn& instruction : std::span(instructions, instruction_count)) {
    U32 current_offset = narrow_cast<U32>(instruction.address);
    ::cs_detail* details = instruction.detail;

    switch (instruction.id) {
      case ::X86_INS_MOV:
      case ::X86_INS_MOVABS: {
        CSS_ASSERT(details->x86.op_count == 2);
        ::cs_x86_op* src = &details->x86.operands[1];
        ::cs_x86_op* dest = &details->x86.operands[0];
        if (dest->type == ::X86_OP_REG) {
          map.registers.store(dest->reg, *src, current_offset);
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
              map.registers.add(dest->reg, increment, current_offset);
            } else {
              map.registers.add(dest->reg, -increment, current_offset);
            }
          }
        }
        break;
      }

      case ::X86_INS_POP: {
        CSS_ASSERT(details->x86.op_count == 1);
        ::cs_x86_op* src = &details->x86.operands[0];
        map.touches.push_back(Stack_Map_Touch{
            .offset = current_offset,
            .entry_rsp_relative_address = get_rsp_adjustment(),
            .byte_count = src->size,
            .access_kind = Stack_Access_Kind::read_only,
        });
        map.registers.add(::X86_REG_RSP, src->size, current_offset);
        break;
      }

      case ::X86_INS_RET:
        map.touches.push_back(Stack_Map_Touch{
            .offset = current_offset,
            .entry_rsp_relative_address = get_rsp_adjustment(),
            .byte_count = 8,
            .access_kind = Stack_Access_Kind::read_only,
        });
        map.registers.add(::X86_REG_RSP, 8, current_offset);
        break;

      case ::X86_INS_PUSH: {
        CSS_ASSERT(details->x86.op_count == 1);
        ::cs_x86_op* src = &details->x86.operands[0];
        map.registers.add(::X86_REG_RSP, -src->size, current_offset);
        map.touches.push_back(Stack_Map_Touch{
            .offset = current_offset,
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
        map.registers.store(dest->reg, map.registers.load(src->mem.base),
                            current_offset);
        map.registers.add(dest->reg, src->mem.disp, current_offset);
        break;
      }

      case ::X86_INS_CALL: {
        // Key: entry_rsp_relative_address
        // Value: instruction_offset
        std::unordered_map<S64, U32> address_to_instruction_offset;

        for (U8 reg = Register_Name::first_register_name;
             reg < Register_Name::max_register_name; ++reg) {
          if (reg == Register_Name::rsp) continue;
          Register_Value& value = map.registers.values[reg];
          bool is_register_likely_updated_for_this_function_call =
              value.last_update_offset >= last_call_offset;
          if (value.kind == Register_Value_Kind::entry_rsp_relative &&
              is_register_likely_updated_for_this_function_call) {
            // NOTE(strager): Using std::unordered_map<>::operator[] is fine
            // because it returns 0 if an entry is missing and
            // value.last_update_offset >= 0.
            U32& existing_touch_instruction_offset =
                address_to_instruction_offset[get_rsp_adjustment_from_value(
                    value)];
            if (existing_touch_instruction_offset < value.last_update_offset) {
              existing_touch_instruction_offset = value.last_update_offset;
            }
          }
        }

        for (auto& [entry_rsp_relative_address, instruction_offset] :
             address_to_instruction_offset) {
          map.touches.push_back(Stack_Map_Touch{
              .offset = instruction_offset,
              .entry_rsp_relative_address = entry_rsp_relative_address,
              .byte_count = (U32)-1,
              .access_kind = Stack_Access_Kind::read_or_write,
          });
        }

        last_call_offset = current_offset;
        break;
      }

      case ::X86_INS_STOSB:
      case ::X86_INS_STOSD:
      case ::X86_INS_STOSQ:
      case ::X86_INS_STOSW: {
        // Examples:
        // stos %eax, (%rdi)
        // rep stos %rax, (%rdi)
        Register_Value dest = map.registers.values[Register_Name::rdi];
        ::cs_x86_op* dest_operand = &details->x86.operands[1];

        std::optional<U32> byte_count;
        if (instruction.detail->x86.prefix[0] == ::X86_PREFIX_REP) {
          Register_Value count = map.registers.values[Register_Name::rcx];
          if (count.kind == Register_Value_Kind::literal) {
            byte_count = count.literal * dest_operand->size;
          }
        } else {
          byte_count = dest_operand->size;
        }

        if (dest.kind == Register_Value_Kind::entry_rsp_relative) {
          map.touches.push_back(Stack_Map_Touch{
              .offset = current_offset,
              .entry_rsp_relative_address = get_rsp_adjustment_from_value(dest),
              .byte_count = byte_count.has_value() ? *byte_count : (U32)-1,
              .access_kind = Stack_Access_Kind::write_only,
          });
        }

        if (byte_count.has_value()) {
          map.registers.add(::X86_REG_RDI, *byte_count, current_offset);
        } else {
          map.registers.store(::X86_REG_RDI,
                              Register_Value::make_unknown(current_offset),
                              current_offset);
        }
        break;
      }

      case ::X86_INS_MOVSB:
      case ::X86_INS_MOVSD:
      case ::X86_INS_MOVSQ:
      case ::X86_INS_MOVSW: {
        // Examples:
        // movs %eax, (%rdi)
        // rep movs %rax, (%rdi)
        Register_Value src = map.registers.values[Register_Name::rsi];
        Register_Value dest = map.registers.values[Register_Name::rdi];
        ::cs_x86_op* dest_operand = &details->x86.operands[1];

        std::optional<U32> byte_count;
        if (instruction.detail->x86.prefix[0] == ::X86_PREFIX_REP) {
          Register_Value count = map.registers.values[Register_Name::rcx];
          if (count.kind == Register_Value_Kind::literal) {
            byte_count = count.literal * dest_operand->size;
          }
        } else {
          byte_count = dest_operand->size;
        }

        if (src.kind == Register_Value_Kind::entry_rsp_relative) {
          map.touches.push_back(Stack_Map_Touch{
              .offset = current_offset,
              .entry_rsp_relative_address = get_rsp_adjustment_from_value(src),
              .byte_count = byte_count.has_value() ? *byte_count : (U32)-1,
              .access_kind = Stack_Access_Kind::read_only,
          });
        }
        if (dest.kind == Register_Value_Kind::entry_rsp_relative) {
          map.touches.push_back(Stack_Map_Touch{
              .offset = current_offset,
              .entry_rsp_relative_address = get_rsp_adjustment_from_value(dest),
              .byte_count = byte_count.has_value() ? *byte_count : (U32)-1,
              .access_kind = Stack_Access_Kind::write_only,
          });
        }

        if (byte_count.has_value()) {
          map.registers.add(::X86_REG_RDI, *byte_count, current_offset);
          map.registers.add(::X86_REG_RSI, *byte_count, current_offset);
        } else {
          map.registers.store(::X86_REG_RDI,
                              Register_Value::make_unknown(current_offset),
                              current_offset);
          map.registers.store(::X86_REG_RSI,
                              Register_Value::make_unknown(current_offset),
                              current_offset);
        }
        break;
      }

      default:
        if (details->x86.op_count == 2) {
          for (U8 operand_index = 0; operand_index < 2; ++operand_index) {
            ::cs_x86_op* operand = &details->x86.operands[operand_index];
            if (operand->type == ::X86_OP_MEM) {
              Register_Value base_address =
                  map.registers.load(operand->mem.base);
              if (base_address.kind ==
                  Register_Value_Kind::entry_rsp_relative) {
                // Examples:
                // mov other_operand, (%rsp)
                // mov (%rsp), other_operand
                // mov -0x10(%rbp), other_operand
                // movzbl (%rsp), other_operand
                map.touches.push_back(Stack_Map_Touch{
                    .offset = current_offset,
                    .entry_rsp_relative_address =
                        get_rsp_adjustment_from_value(base_address) +
                        operand->mem.disp,
                    .byte_count = operand->size,
                    .access_kind =
                        stack_access_kind_from_capstone(operand->access),
                });
              }
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
