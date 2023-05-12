int callee(int a, int b, int c, int d, int e) {
    return a + b + c + d + e;
}

int caller(int a) {
    return a + callee(a + 1, a + 2, a + 3, a + 4, a + 5);
}
