#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/stack-map-touch-group.h>
#include <map>
#include <memory_resource>

namespace cppstacksize {
void Stack_Map_Touch_Groups::set_touches(
    std::span<const Stack_Map_Touch> touches,
    std::span<const Stack_Map_Touch_Location> locations) {
  // See NOTE[touch-locations-size].
  CSS_ASSERT(locations.size() == touches.size());

  // Per-group data used temporarily while building the groups.
  struct Group_Temp_Data {
    explicit Group_Temp_Data(U64 group_index) : group_index(group_index) {}

    // Index into this->groups_.
    U64 group_index;
  };

  std::pmr::monotonic_buffer_resource memory;
  std::pmr::map<Group_Key, Group_Temp_Data> location_to_group_temp(&memory);

  this->groups_.clear();
  for (U64 i = 0; i < touches.size(); ++i) {
    const Stack_Map_Touch& touch = touches[i];
    U32 touched_size = touch.byte_count;
    // TODO(strager): Make byte_count 0 inside the Stack_Map_Touch instead of
    // fixing it up here.
    if (touched_size == (U32)-1) {
      touched_size = 0;
    }
    U32 write_size = touched_size * touches[i].is_write();
    U32 read_size = touched_size * touches[i].is_read();

    auto [existing_it, inserted] = location_to_group_temp.try_emplace(
        this->group_key(locations[i]), this->groups_.size());
    if (inserted) {
      this->groups_.push_back(Group{
          .first_index = i,
          .last_index = i,
          .total_touched_size = touched_size,
          .total_read_size = read_size,
          .total_write_size = write_size,
      });
    } else {
      Group_Temp_Data& group_temp = existing_it->second;
      Group& group = this->groups_.at(group_temp.group_index);
      group.last_index = i;
      group.total_touched_size += touched_size;
      group.total_read_size += read_size;
      group.total_write_size += write_size;
    }
  }
}

std::span<const Stack_Map_Touch_Groups::Group>
Stack_Map_Touch_Groups::raw_groups() const {
  return this->groups_;
}

U64 Stack_Map_Touch_Groups::size() const { return this->groups_.size(); }
}
