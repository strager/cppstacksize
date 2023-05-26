union Empty_Union {};

union Union_With_Int {
  int a;
};

union Union_With_Int_And_Double {
  int a;
  double b;
};

union Forward_Declared_Union;

void f() {
  Empty_Union eu;
  Empty_Union *p_eu = &eu;
  const volatile Empty_Union cveu;

  Union_With_Int uwi;
  uwi.a = 42;
  Union_With_Int *p_uwi = &uwi;

  Union_With_Int_And_Double uwiad;
  uwiad.a = 42;
  uwiad.b = 69.420;
  Union_With_Int_And_Double *p_uwiad = &uwiad;

  Forward_Declared_Union *p_fdu = nullptr;
}
