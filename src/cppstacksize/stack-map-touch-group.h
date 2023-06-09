#pragma once

#include <compare>
#include <cppstacksize/base.h>
#include <cppstacksize/line-tables.h>
#include <span>

namespace cppstacksize {
struct Stack_Map_Touch;

struct Stack_Map_Touch_Location {
  Line_Source_Info line_source_info = Line_Source_Info::out_of_bounds();
  // C string allocated inside touch_data_cache_strings_, or nullptr.
  const char *errors_for_tool_tip = nullptr;
};

// Groups Stack_Map_Touch-s by their source location.
class Stack_Map_Touch_Groups {
 public:
  // A list of Stack_Map_Touch-s grouped by Stack_Map_Touch_Group_Location (file
  // and line number).
  struct Group {
    // The first and last index into touches_ and touch_locations_ with the
    // group's line number (see NOTE[touch-locations-size]).
    //
    // Invariant: group_key(touch_locations_[first_index]) ==
    //            group_key(touch_locations_[last_index ])
    //
    // It is possible that
    // touch_locations_[first_index].line_source_info.line_number is different
    // than touch_locations_[first_index + 1].line_source_info.line_number.
    U64 first_index;
    U64 last_index;

    // Number of bytes touched for each Stack_Map_Touch in this group.
    U64 total_touched_size;

    // Number of bytes read by all Stack_Map_Touch in this group.
    U64 total_read_size;

    // Number of bytes written by all Stack_Map_Touch in this group.
    U64 total_write_size;
  };

  void set_touches(std::span<const Stack_Map_Touch> touches,
                   std::span<const Stack_Map_Touch_Location> locations);

  std::span<const Group> raw_groups() const;
  U64 size() const;

 private:
  struct Group_Key {
    U32 line_number;

    friend std::strong_ordering operator<=>(const Group_Key &,
                                            const Group_Key &) = default;
  };

  static Group_Key group_key(const Stack_Map_Touch_Location &location) {
    // TODO[line-table-file]: Include the file ID.
    return Group_Key{
        .line_number = location.line_source_info.line_number,
    };
  }

  std::vector<Group> groups_;
};
}
