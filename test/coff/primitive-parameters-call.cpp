extern void primitiveParameters(
    unsigned char uc,
    unsigned short us,
    unsigned int ui,
    unsigned long ul,
    unsigned long long ull,

    signed char sc,
    signed short ss,
    signed int si,
    signed long sl,
    signed long long sll,

    char c,
    wchar_t wc,

    float f,
    double d,
    long double ld);

void call() {
    primitiveParameters(
        0,
        0,
        0,
        0,
        0,

        0,
        0,
        0,
        0,
        0,

        'x',
        L'x',

        0.0f,
        0.0,
        0.0);
}
