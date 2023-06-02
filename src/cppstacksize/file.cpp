#include <cppstacksize/file.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <span>
#include <sstream>
#include <string>
#include <utility>

namespace cppstacksize {
Loaded_File Loaded_File::load(const char* path) {
  std::ifstream file(path);
  std::stringstream data_stream;
  data_stream << file.rdbuf();
  if (!file) {
    std::fprintf(stderr, "error: failed to read file %s\n", path);
    std::exit(1);
  }
  return Loaded_File(std::move(data_stream).str());
}

std::span<const U8> Loaded_File::data() const noexcept {
  return std::span<const U8>(reinterpret_cast<const U8*>(this->data_.data()),
                             this->data_.size());
}

Loaded_File::Loaded_File(std::string&& data) : data_(std::move(data)) {}
}
