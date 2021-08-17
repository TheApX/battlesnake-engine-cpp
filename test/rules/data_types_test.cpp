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

// Test that Point is small. It is the main memory consumer.
static_assert(sizeof(Point) == 2);
static_assert(sizeof(Point[10]) == 20);

static_assert(std::is_trivially_constructible<SnakeBody>::value);
static_assert(std::is_trivially_destructible<SnakeBody>::value);
static_assert(std::is_trivially_copyable<SnakeBody>::value);

using ::testing::_;
using ::testing::ElementsAre;
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

TEST(PodTest, PointZeroInitialization) {
  Point point;
  std::memset(&point, 0, sizeof(point));

  EXPECT_THAT(point.x, Eq(0));
  EXPECT_THAT(point.y, Eq(0));
}

TEST(SnakeBodyTest, IncreaseLengthWorks) {
  SnakeBody body{
      .body =
          {
              {},
              {},
              {1, 2},
              {3, 4},
              {5, 6},
          },
      .head_index = 2,
      .length = 3,
  };

  body.IncreaseLength(2);

  EXPECT_THAT(body.head_index, Eq(2));
  EXPECT_THAT(body.length, Eq(5));
  EXPECT_THAT(body.Piece(0), Eq(Point{1, 2}));
  EXPECT_THAT(body.Piece(1), Eq(Point{3, 4}));
  EXPECT_THAT(body.Piece(2), Eq(Point{5, 6}));
  EXPECT_THAT(body.Piece(3), Eq(Point{5, 6}));
  EXPECT_THAT(body.Piece(4), Eq(Point{5, 6}));
}

TEST(SnakeBodyTest, IncreaseLengthWrapsAround) {
  SnakeBody body{
      .body = {},
      .head_index = SnakeBody::kMaxSnakeBodyLen - 2,
      .length = 3,
  };
  body.Piece(0) = Point{1, 2};
  body.Piece(1) = Point{3, 4};
  body.Piece(2) = Point{5, 6};

  body.IncreaseLength(2);

  EXPECT_THAT(body.head_index, Eq(SnakeBody::kMaxSnakeBodyLen - 2));
  EXPECT_THAT(body.length, Eq(5));
  EXPECT_THAT(body.Piece(0), Eq(Point{1, 2}));
  EXPECT_THAT(body.Piece(1), Eq(Point{3, 4}));
  EXPECT_THAT(body.Piece(2), Eq(Point{5, 6}));
  EXPECT_THAT(body.Piece(3), Eq(Point{5, 6}));
  EXPECT_THAT(body.Piece(4), Eq(Point{5, 6}));

  EXPECT_THAT(&body.Piece(0) - &body.body[0],
              Eq(SnakeBody::kMaxSnakeBodyLen - 2));
  EXPECT_THAT(&body.Piece(1) - &body.body[0],
              Eq(SnakeBody::kMaxSnakeBodyLen - 1));
  EXPECT_THAT(&body.Piece(2) - &body.body[0], Eq(0));
  EXPECT_THAT(&body.Piece(3) - &body.body[0], Eq(1));
  EXPECT_THAT(&body.Piece(4) - &body.body[0], Eq(2));
}

TEST(SnakeBodyTest, MoveToWorks) {
  SnakeBody body{
      .body =
          {
              {},
              {},
              {1, 2},
              {3, 4},
              {5, 6},
          },
      .head_index = 2,
      .length = 3,
  };

  body.MoveTo(Move::Left);

  EXPECT_THAT(body.head_index, Eq(1));
  EXPECT_THAT(body.length, Eq(3));
  EXPECT_THAT(body.Piece(0), Eq(Point{0, 2}));
  EXPECT_THAT(body.Piece(1), Eq(Point{1, 2}));
  EXPECT_THAT(body.Piece(2), Eq(Point{3, 4}));
}

TEST(SnakeBodyTest, MoveToWrapsAround) {
  SnakeBody body{
      .body =
          {
              {1, 2},
              {3, 4},
              {5, 6},
          },
      .head_index = 0,
      .length = 3,
  };

  body.MoveTo(Move::Left);

  EXPECT_THAT(body.head_index, Eq(SnakeBody::kMaxSnakeBodyLen - 1));
  EXPECT_THAT(body.length, Eq(3));
  EXPECT_THAT(body.Piece(0), Eq(Point{0, 2}));
  EXPECT_THAT(body.Piece(1), Eq(Point{1, 2}));
  EXPECT_THAT(body.Piece(2), Eq(Point{3, 4}));

  EXPECT_THAT(&body.Piece(0) - &body.body[0],
              Eq(SnakeBody::kMaxSnakeBodyLen - 1));
  EXPECT_THAT(&body.Piece(1) - &body.body[0], Eq(0));
  EXPECT_THAT(&body.Piece(2) - &body.body[0], Eq(1));
}

TEST(SnakeBodyTest, Iterate) {
  SnakeBody body{
      .body = {},
      .head_index = SnakeBody::kMaxSnakeBodyLen - 2,
      .length = 3,
  };
  body.Piece(0) = Point{1, 2};
  body.Piece(1) = Point{3, 4};
  body.Piece(2) = Point{5, 6};

  int n = 0;
  for (Point& p : body) {
    EXPECT_THAT(p, Eq(body.Piece(n)));
    ++n;
  }
}

TEST(ObjectSizesTest, ObjectSizes) {
  EXPECT_THAT(sizeof(Point), Eq(2));

  int body_array_size = SnakeBody::kMaxSnakeBodyLen * sizeof(Point);
  int padding = (-body_array_size) % sizeof(int);

  EXPECT_THAT(sizeof(SnakeBody),
              Eq(body_array_size  // Points array
                 + sizeof(int)    // head_index
                 + sizeof(int)    // length
                 + padding        // padding
                 ));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
