#pragma once

#include <cppstacksize/file.h>
#include <cppstacksize/reader.h>

namespace cppstacksize {
class Example_File {
 public:
  // 'relative_path' is relative to the test/ directory.
  explicit Example_File(const char* relative_path)
      : file_(Loaded_File::load(full_path(relative_path).c_str())),
        reader_(file_.data()) {}

  Example_File(const Example_File&) = delete;
  Example_File& operator=(const Example_File&) = delete;

  std::span<const U8> data() { return this->file_.data(); }

  const Span_Reader& reader() { return this->reader_; }

  Loaded_File loaded_file() && { return std::move(this->file_); }

 private:
  static std::string full_path(const char* relative_path);

  Loaded_File file_;
  Span_Reader reader_;
};
}
