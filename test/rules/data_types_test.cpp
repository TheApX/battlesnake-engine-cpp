#include "battlesnake/rules/data_types.h"

#include <cstring>
#include <type_traits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace rules {

namespace {

static_assert(std::is_trivially_constructible<SnakeId>::value);
static_assert(std::is_trivially_destructible<SnakeId>::value);
static_assert(std::is_trivially_copyable<SnakeId>::value);

static_assert(std::is_trivially_constructible<Move>::value);
static_assert(std::is_trivially_destructible<Move>::value);
static_assert(std::is_trivially_copyable<Move>::value);

static_assert(std::is_trivially_constructible<EliminatedCause>::value);
static_assert(std::is_trivially_destructible<EliminatedCause>::value);
static_assert(std::is_trivially_copyable<EliminatedCause>::value);

static_assert(std::is_trivially_constructible<Point>::value);
static_assert(std::is_trivially_destructible<Point>::value);
static_assert(std::is_trivially_copyable<Point>::value);

using ::testing::Eq;

TEST(StringPoolTest, MultipleInserts) {
  StringPool pool;
  StringWrapper a = pool.Add("abc");
  StringWrapper b = pool.Add("abc");

  EXPECT_THAT(pool.Size(), Eq(1));
  EXPECT_THAT(a, Eq(b));
  EXPECT_THAT(a.value, Eq(b.value));
}

TEST(PodTest, SnakeIdZeroInitialization) {
  SnakeId snake_id;
  std::memset(&snake_id, 0, sizeof(snake_id));
  EXPECT_THAT(snake_id, Eq(""));
}

TEST(PodTest, MoveZeroInitialization) {
  Move move;
  std::memset(&move, 0, sizeof(move));
  EXPECT_THAT(move, Eq(Move::Unknown));
}

TEST(PodTest, EliminatedCauseZeroInitialization) {
  EliminatedCause eliminated_cause;
  std::memset(&eliminated_cause, 0, sizeof(eliminated_cause));

  EXPECT_THAT(eliminated_cause.cause, Eq(EliminatedCause::NotEliminated));
  EXPECT_THAT(eliminated_cause.by_id, Eq(""));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
