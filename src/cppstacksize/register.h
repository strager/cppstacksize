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
  first_register_name,

  rax = first_register_name,
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

  // TODO(strager): Rename.
  max_register_name,
};

struct Register_Value {
  Register_Value_Kind kind;
  U32 last_update_offset;

  union {
    // If kind == Register_Value_Kind::literal:
    U64 literal;

    // If kind == Register_Value_Kind::entry_rsp_relative:
    U64 entry_rsp_relative_offset;
  };

  static Register_Value make_uninitialized() { return make_unknown((U32)-1); }

  static Register_Value make_unknown(U32 last_update_offset) {
    Register_Value value;
    value.kind = Register_Value_Kind::unknown;
    value.last_update_offset = last_update_offset;
    return value;
  }

  static Register_Value make_entry_rsp_relative(U64 offset) {
    Register_Value value;
    value.kind = Register_Value_Kind::entry_rsp_relative;
    value.entry_rsp_relative_offset = offset;
    return value;
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

  static Register_Value make_literal(unsigned long long v) {
    Register_Value value;
    value.kind = Register_Value_Kind::literal;
    value.literal = static_cast<U64>(v);
    return value;
  }

  friend bool operator==(const Register_Value&, const Register_Value&);
  friend bool operator!=(const Register_Value&, const Register_Value&);

 private:
  explicit Register_Value() = default;
};

struct Register_File {
  explicit Register_File();

  Register_Value values[Register_Name::max_register_name];

  void store(/*::x86_reg*/ U32 dest, const ::cs_x86_op& src, U32 update_offset);
  void store(/*::x86_reg*/ U32 dest, const Register_Value& src,
             U32 update_offset);

  Register_Value load(/*::x86_reg*/ U32 src);
  Register_Value load(const ::cs_x86_op& src);

  void add(/*::x86_reg*/ U32 dest, U64 addend, U32 update_offset);
};

std::ostream& operator<<(std::ostream& out, const Register_Value&);
}

#endif
