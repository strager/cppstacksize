#include <cppstacksize/asm-stack-map.h>
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

  std::ifstream s(argv[1]);
  std::stringstream machine_code_stream;
  machine_code_stream << s.rdbuf();
  if (!s) {
    std::fprintf(stderr, "error: failed to read file %s\n", argv[1]);
    std::exit(1);
  }

  std::string machine_code = std::move(machine_code_stream).str();
  std::span<const U8> machine_code_bytes(
      reinterpret_cast<const U8*>(machine_code.data()), machine_code.size());
  Stack_Map map = analyze_x86_64_stack_map(machine_code_bytes);
  for (Stack_Map_Touch& touch : map.touches) {
    std::cout << touch << '\n';
  }
}
