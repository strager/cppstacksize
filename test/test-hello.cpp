#include <cppstacksize/hello.h>
#include <gtest/gtest.h>

TEST(Test_Hello, hello) { ASSERT_EQ(2 + 2, hello()); }
