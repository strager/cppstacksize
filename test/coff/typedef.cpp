typedef int Int_Typedef;
using Int_Using = int;

typedef struct Struct {
    int x;
} Struct_Typedef, *Struct_Typedef_Pointer;

typedef union Union {
    int x;
    double y;
} Union_Typedef, *Union_Typedef_Pointer;

typedef enum Enum {
    A,
} Enum_Typedef;

void f() {
    Int_Typedef it = 0;
    Int_Using iu = 0;

    Struct_Typedef st = {42};
    Struct_Typedef *p_st = &st;
    Struct_Typedef_Pointer stp = &st;

    Union_Typedef ut;
    ut.x = 42;
    Union_Typedef *p_ut = &ut;
    Union_Typedef_Pointer utp = &ut;

    Enum_Typedef et = A;
}
