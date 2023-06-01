#ifndef CPPSTACKSIZE_C_API_H
#define CPPSTACKSIZE_C_API_H

#include <stddef.h>
#include <stdint.h>

extern "C" {
typedef struct CSS_Analyzer CSS_Analyzer;

enum CSS_Stack_Map_Entry_Flag {
  CSS_STACK_MAP_ENTRY_READ = 1 << 0,
  CSS_STACK_MAP_ENTRY_WRITE = 1 << 1,
};

typedef struct CSS_Stack_Map_Entry {
  int32_t entry_rsp_relative_address;
  uint32_t byte_count;
  uint32_t instruction_offset;
  uint32_t flags;
} CSS_Stack_Map_Entry;

CSS_Analyzer* css_create_analyzer_x86_64();
void css_destroy_analyzer(CSS_Analyzer*);

void css_set_machine_code(CSS_Analyzer*, const void* data, size_t size);

void css_get_stack_map(CSS_Analyzer*, const CSS_Stack_Map_Entry** out_entries,
                       size_t* out_entry_count);
}

#endif
