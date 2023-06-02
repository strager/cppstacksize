#ifndef CPPSTACKSIZE_EXAMPLE_FILE_H
#define CPPSTACKSIZE_EXAMPLE_FILE_H

#include <cppstacksize/file.h>
#include <cppstacksize/reader.h>

namespace cppstacksize {
class Example_File {
 public:
  // 'relative_path' is relative to the test/ directory.
  explicit Example_File(const char* relative_path)
      : file_(Loaded_File::load(full_path(relative_path).c_str())),
        reader_(file_.data()) {}

  Span_Reader& reader() { return this->reader_; }

 private:
  static std::string full_path(const char* relative_path);

  Loaded_File file_;
  Span_Reader reader_;
};
}

#endif
