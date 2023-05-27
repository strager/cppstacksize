#include <capstone/capstone.h>
#include <cppstacksize/asm-stack-map.h>
#include <cassert>

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

  ::cs_insn* instructions;
  U64 base_address = 0;
  U64 instruction_count = ::cs_disasm(handle, code.data(), code.size(),
                                      base_address, 0, &instructions);

  S64 rsp_adjustment = 0;
  Stack_Map map;
  for (::cs_insn& instruction : std::span(instructions, instruction_count)) {
    ::cs_detail* details = instruction.detail;

    switch (instruction.id) {
      case ::X86_INS_SUB: {
        CSS_ASSERT(details->x86.op_count == 2);
        ::cs_x86_op* src = &details->x86.operands[1];
        ::cs_x86_op* dest = &details->x86.operands[0];
        if (dest->type == ::X86_OP_REG && dest->reg == ::X86_REG_RSP) {
          if (src->type == ::X86_OP_IMM) {
            // Examples:
            // sub $0x18, %rsp
            // TODO(strager): Checked addition.
            rsp_adjustment -= src->imm;
          }
        }
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
          ::cs_x86_op* other_operand =
              &details->x86.operands[1 - operand_index];
          map.touches.push_back(Stack_Map_Touch{
              .offset = narrow_cast<U32>(instruction.address),
              .entry_rsp_relative_address = rsp_adjustment + operand->mem.disp,
              .byte_count = other_operand->size,
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
