struct S {
    void v();
    void v_i(int);
    void v_ii(int, int);
    void v_iii(int, int, int);
    void v_iiii(int, int, int, int);
    void v_iiiii(int, int, int, int, int);

    static void static_v();
    static void static_v_i(int);
    static void static_v_ii(int, int);
    static void static_v_iii(int, int, int);
    static void static_v_iiii(int, int, int, int);
    static void static_v_iiiii(int, int, int, int, int);
};

void S::v() {}
void S::v_i(int) {}
void S::v_ii(int, int) {}
void S::v_iii(int, int, int) {}
void S::v_iiii(int, int, int, int) {}
void S::v_iiiii(int, int, int, int, int) {}

void S::static_v() {}
void S::static_v_i(int) {}
void S::static_v_ii(int, int) {}
void S::static_v_iii(int, int, int) {}
void S::static_v_iiii(int, int, int, int) {}
void S::static_v_iiiii(int, int, int, int, int) {}
