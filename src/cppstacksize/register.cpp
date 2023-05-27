#include <array>
#include <capstone/capstone.h>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/base.h>
#include <cppstacksize/register.h>

namespace cppstacksize {
namespace {
constexpr std::array<Register_Name, ::X86_REG_ENDING>
make_capstone_x86_reg_to_register_name() {
  std::array<Register_Name, ::X86_REG_ENDING> names;
  for (Register_Name& name : names) {
    name = Register_Name::max_register_name;
  }
  names[::X86_REG_RAX] = Register_Name::rax;
  names[::X86_REG_RBX] = Register_Name::rbx;
  names[::X86_REG_RCX] = Register_Name::rcx;
  names[::X86_REG_RDX] = Register_Name::rdx;
  names[::X86_REG_RSI] = Register_Name::rsi;
  names[::X86_REG_RDI] = Register_Name::rdi;
  names[::X86_REG_RBP] = Register_Name::rbp;
  names[::X86_REG_R8] = Register_Name::r8;
  names[::X86_REG_R9] = Register_Name::r9;
  names[::X86_REG_R10] = Register_Name::r10;
  names[::X86_REG_R11] = Register_Name::r11;
  names[::X86_REG_R12] = Register_Name::r12;
  names[::X86_REG_R13] = Register_Name::r13;
  names[::X86_REG_R14] = Register_Name::r14;
  names[::X86_REG_R15] = Register_Name::r15;
  return names;
}

constexpr std::array<Register_Name, ::X86_REG_ENDING>
    capstone_x86_reg_to_register_name =
        make_capstone_x86_reg_to_register_name();
}

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
      default: {
        Register_Name name = capstone_x86_reg_to_register_name[dest];
        if (name == Register_Name::max_register_name) {
          // TODO(strager)
        } else {
          this->values[name] = Register_Value::make_literal(src.imm);
        }
        break;
      }

      case ::X86_REG_EAX:
        this->values[Register_Name::rax] =
            Register_Value::make_literal(src.imm);
        break;

      case ::X86_REG_AH:
      case ::X86_REG_AL:
      case ::X86_REG_AX: {
        Register_Value old_value = this->values[Register_Name::rax];
        switch (old_value.kind) {
          case Register_Value_Kind::literal: {
            U64 value = old_value.literal;
            switch (static_cast<::x86_reg>(dest)) {
              case ::X86_REG_AX:
                value = (value & ~U64(0xffff)) | src.imm;
                break;
              case ::X86_REG_AL:
                value = (value & ~U64(0xff)) | src.imm;
                break;
              case ::X86_REG_AH:
                value = (value & ~U64(0xff00)) | (src.imm << 8);
                break;
              default:
                __builtin_unreachable();
            }
            this->values[Register_Name::rax].literal = value;
            break;
          }
          default:
            this->values[Register_Name::rax] = Register_Value();
            break;
        }
        break;
      }
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
