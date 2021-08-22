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
static_assert(std::is_trivial<SnakeId>::value);

static_assert(std::is_trivially_constructible<Move>::value);
static_assert(std::is_trivially_destructible<Move>::value);
static_assert(std::is_trivially_copyable<Move>::value);
static_assert(std::is_trivial<Move>::value);

static_assert(std::is_trivially_constructible<EliminatedCause>::value);
static_assert(std::is_trivially_destructible<EliminatedCause>::value);
static_assert(std::is_trivially_copyable<EliminatedCause>::value);
static_assert(std::is_trivial<EliminatedCause>::value);

static_assert(std::is_trivially_constructible<Point>::value);
static_assert(std::is_trivially_destructible<Point>::value);
static_assert(std::is_trivially_copyable<Point>::value);
static_assert(std::is_trivial<Point>::value);

// Test that Point is small. It is the main memory consumer.
static_assert(sizeof(Point) == 2);
static_assert(sizeof(Point[10]) == 20);

static_assert(std::is_trivially_constructible<SnakeBody>::value);
static_assert(std::is_trivially_destructible<SnakeBody>::value);
static_assert(std::is_trivially_copyable<SnakeBody>::value);
static_assert(std::is_trivial<SnakeBody>::value);

static_assert(std::is_trivially_constructible<Snake>::value);
static_assert(std::is_trivially_destructible<Snake>::value);
static_assert(std::is_trivially_copyable<Snake>::value);
static_assert(std::is_trivial<Snake>::value);

static_assert(std::is_trivially_constructible<BoardState>::value);
static_assert(std::is_trivially_destructible<BoardState>::value);
static_assert(std::is_trivially_copyable<BoardState>::value);
static_assert(std::is_trivial<BoardState>::value);

using ::testing::_;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Le;

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
  EXPECT_THAT(move, Eq(Move::Up));
}

TEST(PodTest, EliminatedCauseZeroInitialization) {
  EliminatedCause eliminated_cause;
  std::memset(&eliminated_cause, 0, sizeof(eliminated_cause));

  EXPECT_THAT(eliminated_cause.cause, Eq(EliminatedCause::NotEliminated));
  EXPECT_THAT(eliminated_cause.by_id, Eq(""));
}

TEST(PodTest, PointZeroInitialization) {
  Point point;
  std::memset(&point, 0, sizeof(point));

  EXPECT_THAT(point.x, Eq(0));
  EXPECT_THAT(point.y, Eq(0));
}

TEST(SnakeBodyTest, CreateWorks) {
  SnakeBody body = SnakeBody::Create({
      {1, 2},
      {2, 2},
      {2, 3},
      {1, 3},
      {0, 3},
      {0, 2},
  });

  EXPECT_THAT(body, ElementsAreArray({
                        Point{1, 2},
                        Point{2, 2},
                        Point{2, 3},
                        Point{1, 3},
                        Point{0, 3},
                        Point{0, 2},
                    }));
}

TEST(SnakeBodyTest, IncreaseLengthWorks) {
  SnakeBody body = SnakeBody::Create({
      {1, 2},
      {2, 2},
      {2, 3},
  });

  body.IncreaseLength(2);

  EXPECT_THAT(body, ElementsAreArray({
                        Point{1, 2},
                        Point{2, 2},
                        Point{2, 3},
                        Point{2, 3},
                        Point{2, 3},
                    }));
}

TEST(SnakeBodyTest, MoveToWorks) {
  SnakeBody body = SnakeBody::Create({
      {1, 2},
      {2, 2},
      {2, 3},
  });

  body.MoveTo(Move::Left);

  EXPECT_THAT(body, ElementsAreArray({
                        Point{0, 2},
                        Point{1, 2},
                        Point{2, 2},
                    }));
  EXPECT_THAT(body.total_length, Eq(3));
  EXPECT_THAT(body.moves_length, Eq(2));
}

TEST(ObjectSizesTest, ObjectSizes) {
  EXPECT_THAT(sizeof(Point), Eq(2));

  // Check that SnakeBody array uses 2 bits per body piece, plus const
  // threshold.
  int body_array_size = kMaxSnakeBodyLen / 4;
  EXPECT_THAT(sizeof(SnakeBody), Le(body_array_size + 40));

  // This is just for monitoring total size of BoardState. Update as needed.
  EXPECT_THAT(sizeof(BoardState), Eq(3352));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake