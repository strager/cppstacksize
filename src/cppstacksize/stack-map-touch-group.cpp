#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/sparse-bit-set.h>
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

    Sparse_Bit_Set read_set;
    Sparse_Bit_Set write_set;
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

    auto [existing_it, inserted] = location_to_group_temp.try_emplace(
        this->group_key(locations[i]), this->groups_.size());
    Group_Temp_Data& group_temp = existing_it->second;
    if (inserted) {
      this->groups_.push_back(Group{
          .first_index = i,
          .last_index = i,
          .total_read_size = 0,
          .total_write_size = 0,
      });
    } else {
      Group& group = this->groups_.at(group_temp.group_index);
      group.last_index = i;
    }
    if (touch.is_read()) {
      group_temp.read_set.set_range(touch.entry_rsp_relative_address,
                                    touched_size);
    }
    if (touch.is_write()) {
      group_temp.write_set.set_range(touch.entry_rsp_relative_address,
                                     touched_size);
    }
  }

  for (auto& [_group_key, group_temp] : location_to_group_temp) {
    Group& group = this->groups_.at(group_temp.group_index);
    group.total_read_size = group_temp.read_set.count();
    group.total_write_size = group_temp.write_set.count();
  }
}

std::span<const Stack_Map_Touch_Groups::Group>
Stack_Map_Touch_Groups::raw_groups() const {
  return this->groups_;
}

U64 Stack_Map_Touch_Groups::size() const { return this->groups_.size(); }
}
