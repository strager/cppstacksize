#include <cppstacksize/sparse-bit-set.h>
#include <gtest/gtest.h>

namespace cppstacksize {
namespace {
TEST(Test_Sparse_Bit_Set, new_set_is_empty) {
  Sparse_Bit_Set set;
  EXPECT_EQ(set.count(), 0);
}

TEST(Test_Sparse_Bit_Set, add_one_range_updates_count) {
  {
    Sparse_Bit_Set set;
    set.set_range(10, 2);
    EXPECT_EQ(set.count(), 2);
  }

  {
    Sparse_Bit_Set set;
    set.set_range(3, 9);
    EXPECT_EQ(set.count(), 9);
  }
}

TEST(Test_Sparse_Bit_Set, add_distinct_ranges) {
  {
    Sparse_Bit_Set set;
    set.set_range(10, 2);
    set.set_range(20, 3);
    EXPECT_EQ(set.count(), 2 + 3);
  }

  {
    Sparse_Bit_Set set;
    set.set_range(20, 3);
    set.set_range(10, 2);
    EXPECT_EQ(set.count(), 2 + 3);
  }
}

TEST(Test_Sparse_Bit_Set, add_almost_overlapping_ranges) {
  {
    Sparse_Bit_Set set;
    set.set_range(10, 2);
    set.set_range(12, 3);
    EXPECT_EQ(set.count(), 2 + 3);
  }

  {
    Sparse_Bit_Set set;
    set.set_range(12, 3);
    set.set_range(10, 2);
    EXPECT_EQ(set.count(), 2 + 3);
  }
}

TEST(Test_Sparse_Bit_Set, add_partially_overlapping_ranges) {
  {
    Sparse_Bit_Set set;
    set.set_range(10, 5);
    set.set_range(12, 5);
    EXPECT_EQ(set.count(), 7) << "10,11,12,13,14,15,16";
  }

  {
    Sparse_Bit_Set set;
    set.set_range(12, 5);
    set.set_range(10, 5);
    EXPECT_EQ(set.count(), 7) << "10,11,12,13,14,15,16";
  }
}

TEST(Test_Sparse_Bit_Set, add_duplicate_ranges) {
  Sparse_Bit_Set set;
  set.set_range(10, 5);
  set.set_range(10, 5);
  EXPECT_EQ(set.count(), 5);
}

TEST(Test_Sparse_Bit_Set, add_range_consuming_another) {
  {
    Sparse_Bit_Set set;
    set.set_range(10, 6);
    set.set_range(12, 2);
    EXPECT_EQ(set.count(), 6);
  }

  {
    Sparse_Bit_Set set;
    set.set_range(12, 2);
    set.set_range(10, 6);
    EXPECT_EQ(set.count(), 6);
  }
}
}
}
