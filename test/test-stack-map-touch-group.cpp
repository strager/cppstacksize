#include <cppstacksize/asm-stack-map.h>
#include <cppstacksize/stack-map-touch-group.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_Stack_Map_Touch_Group, one_line_groups_all_together) {
  static constexpr Stack_Map_Touch touches[] = {
      Stack_Map_Touch::read(0, 0x10, 4),
      Stack_Map_Touch::read(1, 0x14, 4),
      Stack_Map_Touch::read(2, 0x18, 8),
  };
  static constexpr Stack_Map_Touch_Location loc = {
      .line_source_info = Line_Source_Info{.line_number = 42},
  };
  static constexpr Stack_Map_Touch_Location locations[] = {loc, loc, loc};

  Stack_Map_Touch_Groups groups;
  groups.set_touches(touches, locations);
  ASSERT_EQ(groups.size(), 1);
  EXPECT_EQ(groups.raw_groups()[0].total_read_size, 4 + 4 + 8);
  EXPECT_EQ(groups.raw_groups()[0].first_index, 0);
  EXPECT_EQ(groups.raw_groups()[0].last_index, 2);
}

TEST(Test_Stack_Map_Touch_Group, reads_are_tracked_separately_from_writes) {
  {
    static constexpr Stack_Map_Touch touches[] = {
        Stack_Map_Touch::read(0, 0x10, 4),
        Stack_Map_Touch::write(1, 0x18, 8),
    };
    static constexpr Stack_Map_Touch_Location locations[] = {
        {.line_source_info = Line_Source_Info{.line_number = 42}},
        {.line_source_info = Line_Source_Info{.line_number = 69}},
    };

    Stack_Map_Touch_Groups groups;
    groups.set_touches(touches, locations);
    ASSERT_EQ(groups.size(), 2);

    EXPECT_EQ(groups.raw_groups()[0].total_read_size, 4);
    EXPECT_EQ(groups.raw_groups()[0].total_write_size, 0);

    EXPECT_EQ(groups.raw_groups()[1].total_read_size, 0);
    EXPECT_EQ(groups.raw_groups()[1].total_write_size, 8);
  }

  {
    static constexpr Stack_Map_Touch touches[] = {
        Stack_Map_Touch::read(0, 0x10, 4),
        Stack_Map_Touch::write(1, 0x18, 8),
    };
    static constexpr Stack_Map_Touch_Location locations[] = {
        {.line_source_info = Line_Source_Info{.line_number = 42}},
        {.line_source_info = Line_Source_Info{.line_number = 42}},
    };

    Stack_Map_Touch_Groups groups;
    groups.set_touches(touches, locations);
    ASSERT_EQ(groups.size(), 1);
    EXPECT_EQ(groups.raw_groups()[0].total_read_size, 4);
    EXPECT_EQ(groups.raw_groups()[0].total_write_size, 8);
  }
}

TEST(Test_Stack_Map_Touch_Group,
     touches_on_separate_lines_are_not_grouped_together) {
  static constexpr Stack_Map_Touch touches[] = {
      Stack_Map_Touch::read(0, 0x10, 4),
      Stack_Map_Touch::read(1, 0x14, 4),
      Stack_Map_Touch::read(2, 0x18, 8),
  };
  static constexpr Stack_Map_Touch_Location locations[] = {
      {.line_source_info = Line_Source_Info{.line_number = 7}},
      {.line_source_info = Line_Source_Info{.line_number = 42}},
      {.line_source_info = Line_Source_Info{.line_number = 69}},
  };

  Stack_Map_Touch_Groups groups;
  groups.set_touches(touches, locations);
  ASSERT_EQ(groups.size(), 3);

  // Line 7:
  EXPECT_EQ(groups.raw_groups()[0].total_read_size, 4);
  EXPECT_EQ(groups.raw_groups()[0].first_index, 0);
  EXPECT_EQ(groups.raw_groups()[0].last_index, 0);

  // Line 42:
  EXPECT_EQ(groups.raw_groups()[1].total_read_size, 4);
  EXPECT_EQ(groups.raw_groups()[1].first_index, 1);
  EXPECT_EQ(groups.raw_groups()[1].last_index, 1);

  // Line 69:
  EXPECT_EQ(groups.raw_groups()[2].total_read_size, 8);
  EXPECT_EQ(groups.raw_groups()[2].first_index, 2);
  EXPECT_EQ(groups.raw_groups()[2].last_index, 2);
}

TEST(Test_Stack_Map_Touch_Group,
     touches_with_out_of_bounds_line_info_are_grouped_together) {
  static constexpr Stack_Map_Touch touches[] = {
      Stack_Map_Touch::read(0, 0x10, 4),
      Stack_Map_Touch::read(1, 0x14, 4),
      Stack_Map_Touch::read(2, 0x18, 8),
  };
  static constexpr Stack_Map_Touch_Location locations[] = {
      {.line_source_info = Line_Source_Info::out_of_bounds()},
      {.line_source_info = Line_Source_Info{.line_number = 42}},
      {.line_source_info = Line_Source_Info::out_of_bounds()},
  };

  Stack_Map_Touch_Groups groups;
  groups.set_touches(touches, locations);
  ASSERT_EQ(groups.size(), 2);

  // Out of bounds:
  EXPECT_EQ(groups.raw_groups()[0].total_read_size, 4 + 8);
  EXPECT_EQ(groups.raw_groups()[0].first_index, 0);
  EXPECT_EQ(groups.raw_groups()[0].last_index, 2);

  // Line 42:
  EXPECT_EQ(groups.raw_groups()[1].total_read_size, 4);
  EXPECT_EQ(groups.raw_groups()[1].first_index, 1);
  EXPECT_EQ(groups.raw_groups()[1].last_index, 1);
}
}
}
