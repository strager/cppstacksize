#ifndef CPPSTACKSIZE_REGISTER_H
#define CPPSTACKSIZE_REGISTER_H

#include <cppstacksize/base.h>

typedef struct cs_x86_op cs_x86_op;

namespace cppstacksize {
enum class Register_Value_Kind : U8 {
  unknown,
  literal,
  entry_rsp_relative,
};

enum Register_Name : U8 {
  rax,

  max_register_name,
};

struct Register_Value {
  Register_Value_Kind kind = Register_Value_Kind::unknown;

  union {
    // If kind == Register_Value_Kind::literal:
    U64 literal;

    // If kind == Register_Value_Kind::entry_rsp_relative:
    S64 entry_rsp_relative_offset;
  };

  static Register_Value make_literal(S64 value) {
    return make_literal(static_cast<U64>(value));
  }

  static Register_Value make_literal(U64 value) {
    return Register_Value{
        .kind = Register_Value_Kind::literal,
        .literal = value,
    };
  }
};

struct Register_File {
  Register_Value values[Register_Name::max_register_name];

  void store(/*::x86_reg*/ U32 dest, const ::cs_x86_op& src);

  Register_Value load(/*::x86_reg*/ U32 src);
  Register_Value load(const ::cs_x86_op& src);
};
}

#endif
