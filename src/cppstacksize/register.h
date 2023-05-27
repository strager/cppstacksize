#ifndef CPPSTACKSIZE_REGISTER_H
#define CPPSTACKSIZE_REGISTER_H

#include <cppstacksize/base.h>
#include <iosfwd>

typedef struct cs_x86_op cs_x86_op;

namespace cppstacksize {
enum class Register_Value_Kind : U8 {
  unknown,
  literal,
  entry_rsp_relative,
};

enum Register_Name : U8 {
  rax,
  rbx,
  rcx,
  rdx,
  rsi,
  rdi,
  rbp,
  rsp,
  r8,
  r9,
  r10,
  r11,
  r12,
  r13,
  r14,
  r15,

  max_register_name,
};

struct Register_Value {
  Register_Value_Kind kind = Register_Value_Kind::unknown;

  union {
    // If kind == Register_Value_Kind::literal:
    U64 literal;

    // If kind == Register_Value_Kind::entry_rsp_relative:
    U64 entry_rsp_relative_offset;
  };

  static Register_Value make_entry_rsp_relative(U64 offset) {
    return Register_Value{
        .kind = Register_Value_Kind::entry_rsp_relative,
        .entry_rsp_relative_offset = offset,
    };
  }

  static Register_Value make_literal(int value) {
    return make_literal(static_cast<U64>(value));
  }

  static Register_Value make_literal(unsigned value) {
    return make_literal(static_cast<U64>(value));
  }

  static Register_Value make_literal(long value) {
    return make_literal(static_cast<U64>(value));
  }

  static Register_Value make_literal(unsigned long value) {
    return make_literal(static_cast<unsigned long long>(value));
  }

  static Register_Value make_literal(long long value) {
    return make_literal(static_cast<U64>(value));
  }

  static Register_Value make_literal(unsigned long long value) {
    return Register_Value{
        .kind = Register_Value_Kind::literal,
        .literal = static_cast<U64>(value),
    };
  }

  friend bool operator==(const Register_Value&, const Register_Value&);
  friend bool operator!=(const Register_Value&, const Register_Value&);
};

struct Register_File {
  Register_Value values[Register_Name::max_register_name];

  void store(/*::x86_reg*/ U32 dest, const ::cs_x86_op& src);

  Register_Value load(/*::x86_reg*/ U32 src);
  Register_Value load(const ::cs_x86_op& src);

  void add(/*::x86_reg*/ U32 dest, U64 addend);
};

std::ostream& operator<<(std::ostream& out, const Register_Value&);
}

#endif
