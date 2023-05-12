extern void callee();

void caller1() {
    callee();
}

void caller2() {
    callee();
    callee();
}
