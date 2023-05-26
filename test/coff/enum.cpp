enum Empty_Enum {};

enum Basic_Enum {
  A,
  B,
  C,
};

enum class Enum_Class {
  A,
  B,
  C,
};

enum class Enum_Class_One_Byte : unsigned char {
  A,
  B,
};

void f() {
  Empty_Enum ee = (Empty_Enum)0;

  Basic_Enum be = B;
  Basic_Enum *p_be = &be;

  Enum_Class ec = Enum_Class::C;

  Enum_Class_One_Byte ecob = Enum_Class_One_Byte::A;
}
