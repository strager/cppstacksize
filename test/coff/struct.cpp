struct Empty_Struct {};

struct Struct_With_One_Int {
  int a;
};

struct Struct_With_Two_Ints {
  int a;
  int b;
};

struct Struct_With_Bit_Field {
  unsigned char b0 : 1;
  unsigned char b12 : 2;
};

struct Struct_With_Vtable {
  virtual void g() {}
};

struct Forward_Declared_Struct;

void f() {
  Empty_Struct es;
  Empty_Struct *p_es = &es;
  const volatile Empty_Struct cves;

  Struct_With_One_Int swoi = {42};
  Struct_With_One_Int *p_swoi = &swoi;

  Struct_With_Two_Ints swti = {42, 69};
  Struct_With_Two_Ints *p_swti = &swti;

  Struct_With_Bit_Field swbf = {0, 2};
  Struct_With_Bit_Field *p_swbf = &swbf;

  Struct_With_Vtable swv;
  Struct_With_Vtable *p_swv = &swv;

  Forward_Declared_Struct *p_fds = nullptr;
}
