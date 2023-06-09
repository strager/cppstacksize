#include <array>
#include <capstone/capstone.h>
#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/base.h>
#include <cppstacksize/register.h>

namespace cppstacksize {
namespace {
enum class Register_Piece : U8 {
  low_64,         // e.g. %rax
  low_32,         // e.g. %eax
  low_16,         // e.g. %ax
  low_8,          // e.g. %al
  low_16_high_8,  // e.g. %ah
};

constexpr std::array<Register_Name, ::X86_REG_ENDING>
make_capstone_x86_reg_to_register_name() {
  std::array<Register_Name, ::X86_REG_ENDING> names;
  for (Register_Name& name : names) {
    name = Register_Name::max_register_name;
  }

  auto set = [&names](Register_Name name,
                      std::initializer_list<::x86_reg> registers) {
    for (::x86_reg r : registers) names[r] = name;
  };
  // clang-format off
  set(Register_Name::rax, {::X86_REG_RAX, ::X86_REG_EAX,  ::X86_REG_AX,   ::X86_REG_AL, ::X86_REG_AH});
  set(Register_Name::rbx, {::X86_REG_RBX, ::X86_REG_EBX,  ::X86_REG_BX,   ::X86_REG_BL, ::X86_REG_BH});
  set(Register_Name::rcx, {::X86_REG_RCX, ::X86_REG_ECX,  ::X86_REG_CX,   ::X86_REG_CL, ::X86_REG_CH});
  set(Register_Name::rdx, {::X86_REG_RDX, ::X86_REG_EDX,  ::X86_REG_DX,   ::X86_REG_DL, ::X86_REG_DH});
  set(Register_Name::rsi, {::X86_REG_RSI, ::X86_REG_ESI,  ::X86_REG_SI,   ::X86_REG_SIL});
  set(Register_Name::rdi, {::X86_REG_RDI, ::X86_REG_EDI,  ::X86_REG_DI,   ::X86_REG_DIL});
  set(Register_Name::rbp, {::X86_REG_RBP, ::X86_REG_EBP,  ::X86_REG_BP,   ::X86_REG_BPL});
  set(Register_Name::rsp, {::X86_REG_RSP, ::X86_REG_ESP,  ::X86_REG_SP,   ::X86_REG_SPL});
  set(Register_Name::r8,  {::X86_REG_R8,  ::X86_REG_R8D,  ::X86_REG_R8W,  ::X86_REG_R8B});
  set(Register_Name::r9,  {::X86_REG_R9,  ::X86_REG_R9D,  ::X86_REG_R9W,  ::X86_REG_R9B});
  set(Register_Name::r10, {::X86_REG_R10, ::X86_REG_R10D, ::X86_REG_R10W, ::X86_REG_R10B});
  set(Register_Name::r11, {::X86_REG_R11, ::X86_REG_R11D, ::X86_REG_R11W, ::X86_REG_R11B});
  set(Register_Name::r12, {::X86_REG_R12, ::X86_REG_R12D, ::X86_REG_R12W, ::X86_REG_R12B});
  set(Register_Name::r13, {::X86_REG_R13, ::X86_REG_R13D, ::X86_REG_R13W, ::X86_REG_R13B});
  set(Register_Name::r14, {::X86_REG_R14, ::X86_REG_R14D, ::X86_REG_R14W, ::X86_REG_R14B});
  set(Register_Name::r15, {::X86_REG_R15, ::X86_REG_R15D, ::X86_REG_R15W, ::X86_REG_R15B});
  // clang-format on

  return names;
}

constexpr std::array<Register_Name, ::X86_REG_ENDING>
    capstone_x86_reg_to_register_name =
        make_capstone_x86_reg_to_register_name();

constexpr std::array<Register_Piece, ::X86_REG_ENDING>
make_capstone_x86_reg_to_register_piece() {
  std::array<Register_Piece, ::X86_REG_ENDING> pieces;
  for (Register_Piece& piece : pieces) {
    piece = Register_Piece::low_64;
  }

  for (::x86_reg r :
       {::X86_REG_RAX, ::X86_REG_RBX, ::X86_REG_RCX, ::X86_REG_RDX,  //
        ::X86_REG_RSI, ::X86_REG_RDI, ::X86_REG_RBP, ::X86_REG_RSP,  //
        ::X86_REG_R8, ::X86_REG_R9, ::X86_REG_R10, ::X86_REG_R11,    //
        ::X86_REG_R12, ::X86_REG_R13, ::X86_REG_R14, ::X86_REG_R15}) {
    pieces[r] = Register_Piece::low_64;
  }

  for (::x86_reg r :
       {::X86_REG_EAX, ::X86_REG_EBX, ::X86_REG_ECX, ::X86_REG_EDX,    //
        ::X86_REG_ESI, ::X86_REG_EDI, ::X86_REG_EBP, ::X86_REG_ESP,    //
        ::X86_REG_R8D, ::X86_REG_R9D, ::X86_REG_R10D, ::X86_REG_R11D,  //
        ::X86_REG_R12D, ::X86_REG_R13D, ::X86_REG_R14D, ::X86_REG_R15D}) {
    pieces[r] = Register_Piece::low_32;
  }

  for (::x86_reg r :
       {::X86_REG_AX, ::X86_REG_BX, ::X86_REG_CX, ::X86_REG_DX,        //
        ::X86_REG_SI, ::X86_REG_DI, ::X86_REG_BP, ::X86_REG_SP,        //
        ::X86_REG_R8W, ::X86_REG_R9W, ::X86_REG_R10W, ::X86_REG_R11W,  //
        ::X86_REG_R12W, ::X86_REG_R13W, ::X86_REG_R14W, ::X86_REG_R15W}) {
    pieces[r] = Register_Piece::low_16;
  }

  for (::x86_reg r :
       {::X86_REG_AL, ::X86_REG_BL, ::X86_REG_CL, ::X86_REG_DL,        //
        ::X86_REG_SIL, ::X86_REG_DIL, ::X86_REG_BPL, ::X86_REG_SPL,    //
        ::X86_REG_R8B, ::X86_REG_R9B, ::X86_REG_R10B, ::X86_REG_R11B,  //
        ::X86_REG_R12B, ::X86_REG_R13B, ::X86_REG_R14B, ::X86_REG_R15B}) {
    pieces[r] = Register_Piece::low_8;
  }

  for (::x86_reg r : {::X86_REG_AH, ::X86_REG_BH, ::X86_REG_CH, ::X86_REG_DH}) {
    pieces[r] = Register_Piece::low_16_high_8;
  }

  return pieces;
}

constexpr std::array<Register_Piece, ::X86_REG_ENDING>
    capstone_x86_reg_to_register_piece =
        make_capstone_x86_reg_to_register_piece();
}

bool operator==(const Register_Value& lhs, const Register_Value& rhs) {
  if (lhs.kind != rhs.kind) return false;
  if (lhs.last_update_offset != rhs.last_update_offset) return false;
  switch (lhs.kind) {
    case Register_Value_Kind::unknown:
      return true;
    case Register_Value_Kind::entry_rsp_relative:
      return lhs.entry_rsp_relative_offset == rhs.entry_rsp_relative_offset;
    case Register_Value_Kind::literal:
      return lhs.literal == rhs.literal;
  }
  CSS_UNREACHABLE();
}

bool operator!=(const Register_Value& lhs, const Register_Value& rhs) {
  return !(lhs == rhs);
}

Register_File::Register_File()
    : values{
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
          Register_Value::make_uninitialized(),
      } {}

void Register_File::store(U32 dest, const ::cs_x86_op& src, U32 update_offset) {
  switch (src.type) {
    case ::X86_OP_IMM: {
      // Examples:
      // mov $0, %rax
      // mov $69, %ah
      Register_Name name = capstone_x86_reg_to_register_name[dest];
      if (name == Register_Name::max_register_name) {
        // TODO(strager)
      } else {
        Register_Value& value = this->values[name];
        switch (capstone_x86_reg_to_register_piece[dest]) {
          case Register_Piece::low_32:
          case Register_Piece::low_64:
            value = Register_Value::make_literal(src.imm, update_offset);
            break;

          case Register_Piece::low_16:
          case Register_Piece::low_16_high_8:
          case Register_Piece::low_8:
            Register_Value old_value = this->values[name];
            switch (old_value.kind) {
              case Register_Value_Kind::literal: {
                U64 v = old_value.literal;
                switch (capstone_x86_reg_to_register_piece[dest]) {
                  case Register_Piece::low_16:
                    v = (v & ~U64(0xffff)) | src.imm;
                    break;
                  case Register_Piece::low_8:
                    v = (v & ~U64(0xff)) | src.imm;
                    break;
                  case Register_Piece::low_16_high_8:
                    v = (v & ~U64(0xff00)) | (src.imm << 8);
                    break;
                  default:
                    CSS_UNREACHABLE();
                }
                value.literal = v;
                break;
              }
              default:
                value = Register_Value::make_unknown(update_offset);
                break;
            }
            break;
        }
        value.last_update_offset = update_offset;
      }
      break;
    }

    case ::X86_OP_REG:
      this->store(dest, this->load(src.reg), update_offset);
      break;

    case ::X86_OP_MEM:
      // TODO(strager)
      break;

    case ::X86_OP_INVALID:
      CSS_UNREACHABLE();
      break;
  }
}

void Register_File::store(U32 dest, const Register_Value& src,
                          U32 update_offset) {
  Register_Name name = capstone_x86_reg_to_register_name[dest];
  if (name == Register_Name::max_register_name) {
    // TODO(strager)
  } else {
    Register_Value& value = this->values[name];
    switch (capstone_x86_reg_to_register_piece[dest]) {
      case Register_Piece::low_64:
        value = src;
        break;

      case Register_Piece::low_32:
      case Register_Piece::low_16:
      case Register_Piece::low_16_high_8:
      case Register_Piece::low_8:
        value = Register_Value::make_unknown(update_offset);
        break;
    }
    value.last_update_offset = update_offset;
  }
}

Register_Value Register_File::load(/*::x86_reg*/ U32 src) {
  Register_Name name = capstone_x86_reg_to_register_name[src];
  if (name == Register_Name::max_register_name) {
    return Register_Value::make_uninitialized();
  } else {
    switch (capstone_x86_reg_to_register_piece[src]) {
      case Register_Piece::low_64:
        return this->values[name];
      default:
        // TODO(strager)
        return Register_Value::make_uninitialized();
    }
  }
}

Register_Value Register_File::load(const ::cs_x86_op& src) {
  switch (src.type) {
    case ::X86_OP_IMM:
      // Examples:
      // sub $0x18, %rsp
      // add $0x18, %rsp
      // FIXME(strager): The last_update_offset is incorrect.
      return Register_Value::make_literal(src.imm, (U32)-1);

    case ::X86_OP_REG:
      // Examples:
      // sub %rax, %rsp
      return this->load(src.reg);

    default:
      // TODO(strager)
      return Register_Value::make_uninitialized();
  }
  CSS_UNREACHABLE();
}

void Register_File::add(/*::x86_reg*/ U32 dest, U64 addend, U32 update_offset) {
  Register_Name name = capstone_x86_reg_to_register_name[dest];
  if (name == Register_Name::max_register_name) {
    // TODO(strager)
  } else {
    Register_Value& value = this->values[name];
    switch (capstone_x86_reg_to_register_piece[dest]) {
      case Register_Piece::low_64: {
        switch (value.kind) {
          case Register_Value_Kind::unknown:
            // Do nothing.
            break;
          case Register_Value_Kind::entry_rsp_relative:
            value.entry_rsp_relative_offset += addend;
            break;
          case Register_Value_Kind::literal:
            value.literal += addend;
            break;
        }
        break;
      }

      case Register_Piece::low_32:
      case Register_Piece::low_16:
      case Register_Piece::low_16_high_8:
      case Register_Piece::low_8:
        // TODO(strager)
        break;
    }
    value.last_update_offset = update_offset;
  }
}
}
