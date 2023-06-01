#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/base.h>
#include <cppstacksize/c-api.h>
#include <vector>

using namespace cppstacksize;

extern "C" {
struct CSS_Analyzer {
 public:
  void set_machine_code(std::span<const U8> code) {
    Stack_Map map = analyze_x86_64_stack_map(code);

    this->stack_map_.clear();
    this->stack_map_.reserve(map.touches.size());
    for (Stack_Map_Touch& touch : map.touches) {
      U32 flags = 0;
      switch (touch.access_kind) {
        case Stack_Access_Kind::read_only:
          flags |= CSS_STACK_MAP_ENTRY_READ;
          break;
        case Stack_Access_Kind::write_only:
          flags |= CSS_STACK_MAP_ENTRY_WRITE;
          break;
        case Stack_Access_Kind::read_or_write:
          break;
        case Stack_Access_Kind::read_and_write:
          flags |= CSS_STACK_MAP_ENTRY_READ | CSS_STACK_MAP_ENTRY_WRITE;
          break;
      }
      this->stack_map_.push_back(CSS_Stack_Map_Entry{
          .entry_rsp_relative_address =
              narrow_cast<S32>(touch.entry_rsp_relative_address),
          .byte_count = touch.byte_count,
          .instruction_offset = touch.offset,
          .flags = flags,
      });
    }
  }

  std::span<const CSS_Stack_Map_Entry> get_stack_map() {
    return this->stack_map_;
  }

 private:
  std::vector<CSS_Stack_Map_Entry> stack_map_;
};

CSS_Analyzer* css_create_analyzer_x86_64() { return new CSS_Analyzer(); }

void css_destroy_analyzer(CSS_Analyzer* analyzer) { delete analyzer; }

void css_set_machine_code(CSS_Analyzer* analyzer, const void* data,
                          size_t size) {
  analyzer->set_machine_code(
      std::span<const U8>(reinterpret_cast<const U8*>(data), size));
}

void css_get_stack_map(CSS_Analyzer* analyzer,
                       const CSS_Stack_Map_Entry** out_entries,
                       size_t* out_entry_count) {
  std::span<const CSS_Stack_Map_Entry> entries = analyzer->get_stack_map();
  *out_entries = entries.data();
  *out_entry_count = entries.size();
}
}
