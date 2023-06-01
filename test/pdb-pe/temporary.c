struct S {
  char padding[0x420];
  int x;
};

int local_variable() {
  struct S s = {{0}, 69};
  return s.x;
}

int temporary() { return (struct S){{0}, 69}.x; }

void _start() {}
