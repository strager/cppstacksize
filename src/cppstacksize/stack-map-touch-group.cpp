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

  std::pmr::monotonic_buffer_resource memory;
  std::pmr::map<Group_Key, U64> location_to_group_index(&memory);

  this->groups_.clear();
  for (U64 i = 0; i < touches.size(); ++i) {
    U32 touched_size = touches[i].byte_count;
    // TODO(strager): Make byte_count 0 inside the Stack_Map_Touch instead of
    // fixing it up here.
    if (touched_size == (U32)-1) {
      touched_size = 0;
    }
    auto [existing_it, inserted] = location_to_group_index.try_emplace(
        this->group_key(locations[i]), this->groups_.size());
    if (inserted) {
      this->groups_.push_back(Group{
          .first_index = i,
          .last_index = i,
          .total_touched_size = touched_size,
      });
    } else {
      Group& group = this->groups_.at(existing_it->second);
      group.last_index = i;
      group.total_touched_size += touched_size;
    }
  }
}

std::span<const Stack_Map_Touch_Groups::Group>
Stack_Map_Touch_Groups::raw_groups() const {
  return this->groups_;
}

U64 Stack_Map_Touch_Groups::size() const { return this->groups_.size(); }
}
