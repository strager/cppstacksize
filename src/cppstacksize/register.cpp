#include "cppstacksize/asm-stack-map.h"
#include <capstone/capstone.h>
#include <cppstacksize/base.h>
#include <cppstacksize/register.h>

namespace cppstacksize {
bool operator==(const Register_Value& lhs, const Register_Value& rhs) {
  if (lhs.kind != rhs.kind) return false;
  switch (lhs.kind) {
    case Register_Value_Kind::unknown:
      return true;
    case Register_Value_Kind::entry_rsp_relative:
      return lhs.entry_rsp_relative_offset == rhs.entry_rsp_relative_offset;
    case Register_Value_Kind::literal:
      return lhs.literal == rhs.literal;
  }
  __builtin_unreachable();
}

bool operator!=(const Register_Value& lhs, const Register_Value& rhs) {
  return !(lhs == rhs);
}

void Register_File::store(U32 dest, const ::cs_x86_op& src) {
  if (src.type == ::X86_OP_IMM) {
    // Examples:
    // mov $0, %rax
    // mov $69, %ah
    switch (static_cast<::x86_reg>(dest)) {
      case ::X86_REG_EAX:
      case ::X86_REG_RAX:
        this->values[Register_Name::rax] =
            Register_Value::make_literal(src.imm);
        break;

      case ::X86_REG_AX: {
        Register_Value old_value = this->values[Register_Name::rax];
        switch (old_value.kind) {
          case Register_Value_Kind::literal:
            this->values[Register_Name::rax] = Register_Value::make_literal(
                (old_value.literal & ~U64(0xffff)) | src.imm);
            break;
          default:
            this->values[Register_Name::rax] = Register_Value();
            break;
        }
        break;
      }

      case ::X86_REG_AL: {
        Register_Value old_value = this->values[Register_Name::rax];
        switch (old_value.kind) {
          case Register_Value_Kind::literal:
            this->values[Register_Name::rax] = Register_Value::make_literal(
                (old_value.literal & ~U64(0xff)) | src.imm);
            break;
          default:
            this->values[Register_Name::rax] = Register_Value();
            break;
        }
        break;
      }

      case ::X86_REG_AH: {
        Register_Value old_value = this->values[Register_Name::rax];
        switch (old_value.kind) {
          case Register_Value_Kind::literal:
            this->values[Register_Name::rax] = Register_Value::make_literal(
                (old_value.literal & ~U64(0xff00)) | (src.imm << 8));
            break;
          default:
            this->values[Register_Name::rax] = Register_Value();
            break;
        }
        break;
      }

      default:
        // TODO(strager)
        break;
    }
  }
}

Register_Value Register_File::load(U32 src) {
  if (static_cast<::x86_reg>(src) == ::X86_REG_RAX) {
    return this->values[Register_Name::rax];
  } else {
    // TODO(strager)
    return Register_Value();
  }
}

Register_Value Register_File::load(const ::cs_x86_op& src) {
  switch (src.type) {
    case ::X86_OP_IMM:
      // Examples:
      // sub $0x18, %rsp
      // add $0x18, %rsp
      return Register_Value::make_literal(src.imm);

    case ::X86_OP_REG:
      // Examples:
      // sub %rax, %rsp
      return this->load(src.reg);

    default:
      // TODO(strager)
      return Register_Value();
  }
  __builtin_unreachable();
}
}
