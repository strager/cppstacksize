#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/file.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

int main(int argc, char** argv) {
  using namespace cppstacksize;

  if (argc != 2) {
    std::fprintf(stderr, "usage: %s FILE.bin\n", argv[0]);
    std::exit(2);
  }

  Loaded_File machine_code = Loaded_File::load(argv[1]);
  Stack_Map map = analyze_x86_64_stack_map(machine_code.data());
  for (Stack_Map_Touch& touch : map.touches) {
    std::cout << touch << '\n';
  }
}
