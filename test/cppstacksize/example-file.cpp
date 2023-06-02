#include <cppstacksize/example-file.h>
#include <filesystem>

namespace cppstacksize {
std::string Example_File::full_path(const char* relative_path) {
  // NOTE(strager): __FILE__ should be an absolute path. See
  // HACK[example-file-path].
  std::filesystem::path example_file_h_path(__FILE__);
  example_file_h_path.remove_filename();
  example_file_h_path /= "..";
  example_file_h_path /= relative_path;
  return example_file_h_path.string();
}
}
