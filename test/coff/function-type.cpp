typedef void Function_Void_Typedef();
using Function_Void_Using = void ();

typedef void (*Function_Void_Pointer_Typedef)();
using Function_Void_Pointer_Using = void (*)();

typedef Function_Void_Typedef *Function_Void_Typedef_Pointer_Typedef;
using Function_Void_Using_Pointer_Using = Function_Void_Using *;

void f() {
    void (*fp_v)() = nullptr;
    void (*fp_vi)(int) = nullptr;
    int (*fp_i)() = nullptr;

    Function_Void_Typedef *p_fvt = nullptr;
    Function_Void_Using *p_fvu = nullptr;

    Function_Void_Pointer_Typedef fvpt = nullptr;
    Function_Void_Pointer_Using fvpu = nullptr;

    Function_Void_Typedef_Pointer_Typedef fvtpt = nullptr;
    Function_Void_Using_Pointer_Using fvupu = nullptr;
}
