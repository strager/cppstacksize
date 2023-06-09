#include <cppstacksize/base.h>
#include <memory_resource>
#include <set>

namespace cppstacksize {
class Sparse_Bit_Set {
 public:
  explicit Sparse_Bit_Set()
      : Sparse_Bit_Set(std::pmr::get_default_resource()) {}

  explicit Sparse_Bit_Set(std::pmr::memory_resource* memory)
      : set_bits_(memory) {}

  U64 count() const { return this->set_bits_.size(); }

  void set_range(U64 start, U64 count) {
    for (U64 i = 0; i < count; ++i) {
      this->set_bits_.insert(start + i);
    }
  }

 private:
  // TODO(perf): Make something less slow.
  std::pmr::set<U64> set_bits_;
};
}
