#include <capstone/capstone.h>
#include <cppstacksize/base.h>
#include <cppstacksize/register.h>

namespace cppstacksize {
void Register_File::store(U32 dest, const ::cs_x86_op& src) {
  if (src.type == ::X86_OP_IMM) {
    // Examples:
    // mov $0, %rax
    // mov $69, %ah
    switch (static_cast<::x86_reg>(dest)) {
      case ::X86_REG_RAX:
        this->values[Register_Name::rax] =
            Register_Value::make_literal(src.imm);
        break;

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
