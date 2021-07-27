#include "battlesnake/rules/data_types.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace rules {

namespace {

using ::testing::Eq;

TEST(StringPoolTest, MultipleInserts) {
  StringPool pool;
  std::string_view a = pool.Add("abc");
  std::string_view b = pool.Add("abc");

  EXPECT_THAT(pool.Size(), Eq(1));
  EXPECT_THAT(a, Eq(b));
  EXPECT_THAT(a.data(), Eq(b.data()));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
