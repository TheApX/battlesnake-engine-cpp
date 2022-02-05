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
using ::testing::IsFalse;
using ::testing::IsTrue;
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

TEST(PointTest, LeftNotWrapped) {
  Point point{5, 5};
  Point moved{4, 5};
  EXPECT_THAT(point.Left(), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Left), Eq(moved));
}

TEST(PointTest, RightNotWrapped) {
  Point point{5, 5};
  Point moved{6, 5};
  EXPECT_THAT(point.Right(), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Right), Eq(moved));
}

TEST(PointTest, DownNotWrapped) {
  Point point{5, 5};
  Point moved{5, 4};
  EXPECT_THAT(point.Down(), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Down), Eq(moved));
}

TEST(PointTest, UpNotWrapped) {
  Point point{5, 5};
  Point moved{5, 6};
  EXPECT_THAT(point.Up(), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Up), Eq(moved));
}

TEST(PointTest, LeftWrapped) {
  Point size{5, 5};
  Point point{0, 0};
  Point moved{4, 0};
  EXPECT_THAT(point.Left(&size), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Left, &size), Eq(moved));
}

TEST(PointTest, RightWrapped) {
  Point size{5, 5};
  Point point{4, 4};
  Point moved{0, 4};
  EXPECT_THAT(point.Right(&size), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Right, &size), Eq(moved));
}

TEST(PointTest, DownWrapped) {
  Point size{5, 5};
  Point point{0, 0};
  Point moved{0, 4};
  EXPECT_THAT(point.Down(&size), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Down, &size), Eq(moved));
}

TEST(PointTest, UpWrapped) {
  Point size{5, 5};
  Point point{4, 4};
  Point moved{4, 0};
  EXPECT_THAT(point.Up(&size), Eq(moved));
  EXPECT_THAT(point.Moved(Move::Up, &size), Eq(moved));
}

TEST(DetectMoveTest, LeftNotWrapped) {
  Point point{5, 5};
  Point moved{4, 5};
  EXPECT_THAT(DetectMove(point, moved), Eq(Move::Left));
}

TEST(DetectMoveTest, RightNotWrapped) {
  Point point{5, 5};
  Point moved{6, 5};
  EXPECT_THAT(DetectMove(point, moved), Eq(Move::Right));
}

TEST(DetectMoveTest, DownNotWrapped) {
  Point point{5, 5};
  Point moved{5, 4};
  EXPECT_THAT(DetectMove(point, moved), Eq(Move::Down));
}

TEST(DetectMoveTest, UpNotWrapped) {
  Point point{5, 5};
  Point moved{5, 6};
  EXPECT_THAT(DetectMove(point, moved), Eq(Move::Up));
}

TEST(DetectMoveTest, LeftWrapped) {
  Point size{5, 5};
  Point point{0, 0};
  Point moved{4, 0};
  EXPECT_THAT(DetectMove(point, moved, &size), Eq(Move::Left));
}

TEST(DetectMoveTest, RightWrapped) {
  Point size{5, 5};
  Point point{4, 4};
  Point moved{0, 4};
  EXPECT_THAT(DetectMove(point, moved, &size), Eq(Move::Right));
}

TEST(DetectMoveTest, DownWrapped) {
  Point size{5, 5};
  Point point{0, 0};
  Point moved{0, 4};
  EXPECT_THAT(DetectMove(point, moved, &size), Eq(Move::Down));
}

TEST(DetectMoveTest, UpWrapped) {
  Point size{5, 5};
  Point point{4, 4};
  Point moved{4, 0};
  EXPECT_THAT(DetectMove(point, moved, &size), Eq(Move::Up));
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

TEST(SnakeBodyTest, CreateAndIterationWrappedWorks) {
  Point size{11, 11};
  SnakeBody body = SnakeBody::Create(
      {
          {2, 0},
          {1, 0},
          {0, 0},
          {10, 0},
          {9, 0},
          {8, 0},
      },
      &size);

  // Check that SnakeBody::Create correctly parsed the body.
  EXPECT_THAT(body.NextMove(0), Eq(Move::Left));
  EXPECT_THAT(body.NextMove(1), Eq(Move::Left));
  EXPECT_THAT(body.NextMove(2), Eq(Move::Left));
  EXPECT_THAT(body.NextMove(3), Eq(Move::Left));
  EXPECT_THAT(body.NextMove(4), Eq(Move::Left));

  // Check that iteration works correctly.
  EXPECT_THAT(body, ElementsAreArray({
                        Point{2, 0},
                        Point{1, 0},
                        Point{0, 0},
                        Point{10, 0},
                        Point{9, 0},
                        Point{8, 0},
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

TEST(SnakeBodyTest, TailWorks) {
  SnakeBody body = SnakeBody::Create({
      {1, 2},
      {2, 2},
      {2, 3},
  });

  EXPECT_THAT(body.TailPos(), Eq(Point{2, 3}));
}

TEST(SnakeBodyTest, RealData) {
  auto data = {
      Point{.x = 4, .y = 1},   //
      Point{.x = 5, .y = 1},   //
      Point{.x = 5, .y = 2},   //
      Point{.x = 5, .y = 3},   //
      Point{.x = 5, .y = 4},   //
      Point{.x = 5, .y = 5},   //
      Point{.x = 5, .y = 6},   //
      Point{.x = 5, .y = 7},   //
      Point{.x = 5, .y = 8},   //
      Point{.x = 5, .y = 9},   //
      Point{.x = 5, .y = 10},  //
      Point{.x = 4, .y = 10},  //
  };

  SnakeBody body = SnakeBody::Create(data);

  EXPECT_THAT(body, ElementsAreArray(data));
}

TEST(BoardBitsTest, Get) {
  BoardBits bits{
      .data = {1, 2, 3},
  };

  EXPECT_THAT(bits.Get(0), IsTrue());
  EXPECT_THAT(bits.Get(65), IsTrue());
  EXPECT_THAT(bits.Get(128), IsTrue());
  EXPECT_THAT(bits.Get(129), IsTrue());

  EXPECT_THAT(bits.Get(1), IsFalse());
  EXPECT_THAT(bits.Get(63), IsFalse());
  EXPECT_THAT(bits.Get(64), IsFalse());
  EXPECT_THAT(bits.Get(130), IsFalse());
}

TEST(BoardBitsTest, Set) {
  BoardBits bits{};

  bits.Set(0, true);
  bits.Set(65, true);
  bits.Set(128, true);
  bits.Set(129, true);

  EXPECT_THAT(bits.data[0], Eq(1));
  EXPECT_THAT(bits.data[1], Eq(2));
  EXPECT_THAT(bits.data[2], Eq(3));

  bits.Set(128, false);

  EXPECT_THAT(bits.data[2], Eq(2));
}

TEST(BoardBitsTest, EmptyInit) {
  BoardBits bits{};

  for (int i = 0; i < BoardBits::kMaxBitsSize; ++i) {
    EXPECT_THAT(bits.Get(i), IsFalse()) << "i = " << i;
  }
}

TEST(BoardBitsTest, ZeroInit) {
  BoardBits bits;
  std::memset(&bits, 0, sizeof(bits));

  for (int i = 0; i < BoardBits::kMaxBitsSize; ++i) {
    EXPECT_THAT(bits.Get(i), IsFalse()) << "i = " << i;
  }
}

TEST(BoardBitsViewTest, Get) {
  BoardBits bits{
      .data = {1, 2, 3},
  };
  BoardBitsView view(&bits, 100, 3);

  EXPECT_THAT(view.Get(Point{0, 0}), IsTrue());
  EXPECT_THAT(view.Get(Point{65, 0}), IsTrue());
  EXPECT_THAT(view.Get(Point{28, 1}), IsTrue());
  EXPECT_THAT(view.Get(Point{29, 1}), IsTrue());

  EXPECT_THAT(view.Get(Point{1, 0}), IsFalse());
  EXPECT_THAT(view.Get(Point{63, 0}), IsFalse());
  EXPECT_THAT(view.Get(Point{64, 0}), IsFalse());
  EXPECT_THAT(view.Get(Point{30, 1}), IsFalse());
}

TEST(BoardBitsViewTest, Set) {
  BoardBits bits{};
  BoardBitsView view(&bits, 100, 3);

  view.Set(Point{0, 0}, true);
  view.Set(Point{65, 0}, true);
  view.Set(Point{28, 1}, true);
  view.Set(Point{29, 1}, true);

  EXPECT_THAT(bits.data[0], Eq(1));
  EXPECT_THAT(bits.data[1], Eq(2));
  EXPECT_THAT(bits.data[2], Eq(3));

  view.Set(Point{28, 1}, false);

  EXPECT_THAT(bits.data[2], Eq(2));
}

TEST(BoardBitsViewTest, Fill) {
  BoardBits bits{
      .data =
          {
              0xFFFFFFFFFFFFFFFFull,
              0xFFFFFFFFFFFFFFFFull,
              0xFFFFFFFFFFFFFFFFull,
              0xFFFFFFFFFFFFFFFFull,
          },
  };
  BoardBitsView view(&bits, 100, 3);

  view.Fill({
      Point{0, 0},
      Point{65, 0},
      Point{28, 1},
      Point{29, 1},
  });

  EXPECT_THAT(bits.data[0], Eq(1));
  EXPECT_THAT(bits.data[1], Eq(2));
  EXPECT_THAT(bits.data[2], Eq(3));
  EXPECT_THAT(bits.data[3], Eq(0));
}

TEST(BoardBitsViewTest, RangeFor) {
  BoardBits bits{
      .data = {1, 2, 3},
  };
  BoardBitsView view(&bits, 100, 3);

  std::vector<Point> points;
  for (const Point& p : view) {
    points.push_back(p);
  }

  EXPECT_THAT(points, ElementsAreArray({
                          Point{0, 0},
                          Point{65, 0},
                          Point{28, 1},
                          Point{29, 1},
                      }));
}

TEST(BoardBitsViewTest, ConstRangeFor) {
  BoardBits bits{
      .data = {1, 2, 3},
  };
  BoardBitsViewConst view(&bits, 100, 3);

  std::vector<Point> points;
  for (const Point& p : view) {
    points.push_back(p);
  }

  EXPECT_THAT(points, ElementsAreArray({
                          Point{0, 0},
                          Point{65, 0},
                          Point{28, 1},
                          Point{29, 1},
                      }));
}

TEST(BoardBitsViewTest, Comparison) {
  BoardBits bits_a{
      .data = {1, 2, 3},
  };
  BoardBits bits_b{
      .data = {4, 5, 6},
  };
  EXPECT_THAT(bits_a == bits_b, IsFalse());
  EXPECT_THAT(bits_a != bits_b, IsTrue());
  EXPECT_THAT(bits_a == bits_a, IsTrue());
  EXPECT_THAT(bits_a != bits_a, IsFalse());
}

TEST(BoardBitsViewTest, ProdTest1) {
  BoardBits bits{};
  BoardBitsView view(&bits, 7, 7);

  view.Set(Point{2, 6}, true);
  view.Set(Point{2, 0}, true);
  view.Set(Point{6, 4}, true);
  view.Set(Point{3, 3}, true);

  EXPECT_THAT(view.Get(Point{2, 6}), IsTrue());
  EXPECT_THAT(view.Get(Point{2, 0}), IsTrue());
  EXPECT_THAT(view.Get(Point{6, 4}), IsTrue());
  EXPECT_THAT(view.Get(Point{3, 3}), IsTrue());

  EXPECT_THAT(view.Get(Point{5, 1}), IsFalse());

  std::vector<Point> points;
  for (const Point& p : view) {
    points.push_back(p);
  }

  EXPECT_THAT(points, ElementsAreArray({
                          Point{2, 0},
                          Point{3, 3},
                          Point{6, 4},
                          Point{2, 6},
                      }));
}

TEST(ObjectSizesTest, ObjectSizes) {
  EXPECT_THAT(sizeof(Point), Eq(2));

  // Check that SnakeBody array uses 2 bits per body piece, plus const
  // threshold.
  int body_array_size = kMaxSnakeBodyLen / 4;
  EXPECT_THAT(sizeof(SnakeBody), Le(body_array_size + 40));

  // This is just for monitoring total size of BoardState. Update as needed.
  BoardState board_state;
  EXPECT_THAT(sizeof(board_state), Eq(2232));
  EXPECT_THAT(sizeof(board_state.food), Eq(80));

  EXPECT_THAT(sizeof(BoardBits), Eq(80));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
