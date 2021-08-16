#include "battlesnake/rules/standard_ruleset.h"

#include <algorithm>
#include <initializer_list>

#include "battlesnake/rules/errors.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace rules {

namespace {

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::Lt;
using ::testing::Not;
using ::testing::UnorderedElementsAreArray;

template <class T>
auto ValueBetween(const T& a, const T& b) {
  return AllOf(Ge(a), Lt(b));
}

template <class M>
auto SnakeIdIs(M m) {
  return Field(&Snake::id, m);
}

template <class M>
auto SnakeBodyIs(const M& m) {
  return Field(&Snake::body, m);
}

template <class M>
auto SnakeHealthIs(M m) {
  return Field(&Snake::health, m);
}

template <class A, class B, class C, class D, class E>
auto SnakeIs(const A& m_id, const B& m_body, const C& m_health,
             const D& m_cause, const E& m_eliminated_by) {
  return AllOf(
      Field(&Snake::id, m_id), Field(&Snake::body, m_body),
      Field(&Snake::health, m_health),
      Field(&Snake::eliminated_cause, Field(&EliminatedCause::cause, m_cause)),
      Field(&Snake::eliminated_cause,
            Field(&EliminatedCause::by_id, m_eliminated_by)));
}

template <class A, class B, class C, class D>
auto SnakeIs(const A& m_id, const B& m_body, const C& m_health,
             const D& m_cause) {
  return SnakeIs(m_id, m_body, m_health, m_cause, _);
}

template <class A, class B, class C>
auto SnakeIs(const A& m_id, const B& m_body, const C& m_health) {
  return SnakeIs(m_id, m_body, m_health, _, _);
}

template <class A, class B>
auto SnakeIs(const A& m_id, const B& m_body) {
  return SnakeIs(m_id, m_body, _, _, _);
}

class StandardRulesetTest : public testing::Test {
 protected:
  std::vector<SnakeId> CreateSnakeIds(int n, StringPool& pool) {
    std::vector<SnakeId> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
      result.push_back(pool.Add("Snake" + std::to_string(n)));
    }
    return result;
  }
};

TEST_F(StandardRulesetTest, Sanity) {
  StandardRuleset ruleset;

  BoardState state = ruleset.CreateInitialBoardState(0, 0, {});
  EXPECT_THAT(state.width, Eq(0));
  EXPECT_THAT(state.height, Eq(0));
  EXPECT_THAT(state.snakes, ElementsAre());

  BoardState new_state = ruleset.CreateNextBoardState(state, {}, 0);
  EXPECT_THAT(state.width, Eq(0));
  EXPECT_THAT(state.height, Eq(0));
  EXPECT_THAT(state.snakes, ElementsAre());

  EXPECT_THAT(ruleset.IsGameOver(state), IsTrue());
}

class StandardCreateInitialBoardStateTest : public StandardRulesetTest {
 protected:
  void ExpectBoard(const BoardState& state, int width, int height, int num_food,
                   const std::initializer_list<SnakeId>& ids) {
    EXPECT_THAT(state.width, Eq(width));
    EXPECT_THAT(state.height, Eq(height));
    EXPECT_THAT(state.food.size(), Eq(num_food));
    std::vector<SnakeId> state_snake_ids;
    state_snake_ids.resize(state.snakes.size());
    std::transform(state.snakes.begin(), state.snakes.end(),
                   state_snake_ids.begin(),
                   [](const Snake& snake) -> SnakeId { return snake.id; });
    EXPECT_THAT(state_snake_ids, ElementsAreArray(ids));

    for (const Snake& snake : state.snakes) {
      EXPECT_THAT(snake.body.size(), Eq(3));
    }
  }
};

TEST_F(StandardCreateInitialBoardStateTest, Small1by1) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(1, 1, {pool.Add("one")}), 1, 1, 0,
              {pool.Add("one")});
}

TEST_F(StandardCreateInitialBoardStateTest, Small1by2) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(1, 2, {pool.Add("one")}), 1, 2, 0,
              {pool.Add("one")});
}

TEST_F(StandardCreateInitialBoardStateTest, Small1by4) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(1, 4, {pool.Add("one")}), 1, 4, 1,
              {pool.Add("one")});
}

TEST_F(StandardCreateInitialBoardStateTest, Small2by2) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(2, 2, {pool.Add("one")}), 2, 2, 1,
              {pool.Add("one")});
}

TEST_F(StandardCreateInitialBoardStateTest, NonStandardSize) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(9, 8, {pool.Add("one")}), 9, 8, 1,
              {pool.Add("one")});
}

TEST_F(StandardCreateInitialBoardStateTest, SmallTwoSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoard(
      ruleset.CreateInitialBoardState(2, 2, {pool.Add("one"), pool.Add("two")}),
      2, 2, 0, {pool.Add("one"), pool.Add("two")});
}

TEST_F(StandardCreateInitialBoardStateTest, NoRoom1by1) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(
      ruleset.CreateInitialBoardState(1, 1, {pool.Add("one"), pool.Add("two")}),
      ErrorNoRoomForSnake);
}

TEST_F(StandardCreateInitialBoardStateTest, NoRoom1by2) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(
      ruleset.CreateInitialBoardState(1, 2, {pool.Add("one"), pool.Add("two")}),
      ErrorNoRoomForSnake);
}

TEST_F(StandardCreateInitialBoardStateTest, SmallBoard) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoard(
      ruleset.CreateInitialBoardState(kBoardSizeSmall, kBoardSizeSmall,
                                      {pool.Add("one"), pool.Add("two")}),
      kBoardSizeSmall, kBoardSizeSmall, 3, {pool.Add("one"), pool.Add("two")});
}

class StandardPlaceSnakeTest : public StandardRulesetTest {
 protected:
  void ExpectBoardSnakes(const BoardState& state, int num_snakes) {
    // Check that number of snakes is expected.
    EXPECT_THAT(state.snakes.size(), Eq(num_snakes));

    // Check each snake.
    for (const Snake& snake : state.snakes) {
      // Check that body size is expected.
      EXPECT_THAT(snake.body.size(), Eq(3));

      // Check that head is on an even cell.
      ASSERT_THAT(snake.body.empty(), IsFalse());
      Point prev = snake.body.front();
      EXPECT_THAT((prev.x + prev.x) % 2, Eq(0));

      for (const Point& p : snake.body) {
        // Check that each body piece is within the board.
        EXPECT_THAT(p.x, ValueBetween(0, state.width));
        EXPECT_THAT(p.y, ValueBetween(0, state.height));
        // Check that body pieces are connected or overlap.
        EXPECT_THAT(std::abs(p.x - prev.x) + std::abs(p.y - prev.y), Le(0));
      }
    }
  }
};

TEST_F(StandardPlaceSnakeTest, Small1by1) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardSnakes(
      ruleset.CreateInitialBoardState(1, 1, CreateSnakeIds(1, pool)), 1);
}

TEST_F(StandardPlaceSnakeTest, Small1by1TwoSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(1, 1, CreateSnakeIds(2, pool)),
               ErrorNoRoomForSnake);
}

TEST_F(StandardPlaceSnakeTest, Small1by2TwoSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(1, 2, CreateSnakeIds(2, pool)),
               ErrorNoRoomForSnake);
}

TEST_F(StandardPlaceSnakeTest, Small2by1TwoSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(2, 1, CreateSnakeIds(2, pool)),
               ErrorNoRoomForSnake);
}

TEST_F(StandardPlaceSnakeTest, Small2by2TwoSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardSnakes(
      ruleset.CreateInitialBoardState(2, 2, CreateSnakeIds(2, pool)), 2);
}

TEST_F(StandardPlaceSnakeTest, EnoughSpaceForManySnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardSnakes(
      ruleset.CreateInitialBoardState(5, 10, CreateSnakeIds(25, pool)), 25);
}

TEST_F(StandardPlaceSnakeTest, NotEnoughSpaceForManySnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(5, 10, CreateSnakeIds(26, pool)),
               ErrorNoRoomForSnake);
}

TEST_F(StandardPlaceSnakeTest, KnownSizeSmallOneSnake) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardSnakes(
      ruleset.CreateInitialBoardState(kBoardSizeSmall, kBoardSizeSmall,
                                      CreateSnakeIds(1, pool)),
      1);
}

TEST_F(StandardPlaceSnakeTest, KnownSizeSmallMaxSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardSnakes(
      ruleset.CreateInitialBoardState(kBoardSizeSmall, kBoardSizeSmall,
                                      CreateSnakeIds(8, pool)),
      8);
}

TEST_F(StandardPlaceSnakeTest, KnownSizeSmallTooManySnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(kBoardSizeSmall, kBoardSizeSmall,
                                               CreateSnakeIds(9, pool)),
               ErrorTooManySnakes);
}

TEST_F(StandardPlaceSnakeTest, KnownSizeMediumMaxSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardSnakes(
      ruleset.CreateInitialBoardState(kBoardSizeMedium, kBoardSizeMedium,
                                      CreateSnakeIds(8, pool)),
      8);
}

TEST_F(StandardPlaceSnakeTest, KnownSizeMediumTooManySnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(
                   kBoardSizeMedium, kBoardSizeMedium, CreateSnakeIds(9, pool)),
               ErrorTooManySnakes);
}

TEST_F(StandardPlaceSnakeTest, KnownSizeLargeMaxSnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardSnakes(
      ruleset.CreateInitialBoardState(kBoardSizeLarge, kBoardSizeLarge,
                                      CreateSnakeIds(8, pool)),
      8);
}

TEST_F(StandardPlaceSnakeTest, KnownSizeLargeTooManySnakes) {
  StringPool pool;
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(kBoardSizeLarge, kBoardSizeLarge,
                                               CreateSnakeIds(9, pool)),
               ErrorTooManySnakes);
}

class StandardPlaceFoodTest : public StandardRulesetTest {
 protected:
  void ExpectBoardFood(const BoardState& state, int num_food) {
    EXPECT_THAT(state.food.size(), Eq(num_food));
    for (const Point& food : state.food) {
      EXPECT_THAT(food.x, ValueBetween(0, state.width));
      EXPECT_THAT(food.y, ValueBetween(0, state.height));
    }
  }

  void ExpectFoodAroundSnakes(const BoardState& state) {
    for (const Snake& snake : state.snakes) {
      ASSERT_THAT(snake.body.empty(), IsFalse());
      const Point& head = snake.body.front();

      auto accepted_food_pos = {
          Point{head.x - 1, head.y - 1},
          Point{head.x - 1, head.y + 1},
          Point{head.x + 1, head.y - 1},
          Point{head.x + 1, head.y + 1},
      };

      bool snake_has_food = false;
      for (const Point& pos : accepted_food_pos) {
        for (const Point& food : state.food) {
          if (food == pos) {
            snake_has_food = true;
            break;
          }
        }
      }
      EXPECT_THAT(snake_has_food, IsTrue());
    }
  }
};

TEST_F(StandardPlaceFoodTest, Small1by1) {
  // The only cell is taken by snake, no place for food.
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardFood(
      ruleset.CreateInitialBoardState(1, 1, CreateSnakeIds(1, pool)), 0);
}

TEST_F(StandardPlaceFoodTest, Small1by2) {
  // One cell is taken by snake, but the other one is not even.
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardFood(
      ruleset.CreateInitialBoardState(1, 2, CreateSnakeIds(1, pool)), 0);
}

TEST_F(StandardPlaceFoodTest, ManySnakesMuchSpace) {
  // Many randomly placed snakes, food for everybody.
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardFood(
      ruleset.CreateInitialBoardState(101, 202, CreateSnakeIds(17, pool)), 17);
}

TEST_F(StandardPlaceFoodTest, AllFreeSpaceFilledIn) {
  // Many randomly placed snakes, space for some food, but not for everybody.
  StringPool pool;
  StandardRuleset ruleset;
  ExpectBoardFood(
      ruleset.CreateInitialBoardState(10, 20, CreateSnakeIds(60, pool)), 40);
}

TEST_F(StandardPlaceFoodTest, KnownSizeSmall) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StringPool pool;
  StandardRuleset ruleset;
  BoardState board_state = ruleset.CreateInitialBoardState(
      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(3, pool));

  ExpectBoardFood(board_state, 4);
  ExpectFoodAroundSnakes(board_state);
}

TEST_F(StandardPlaceFoodTest, KnownSizeMiddlle) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StringPool pool;
  StandardRuleset ruleset;
  BoardState board_state = ruleset.CreateInitialBoardState(
      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(8, pool));

  ExpectBoardFood(board_state, 9);
  ExpectFoodAroundSnakes(board_state);
}

TEST_F(StandardPlaceFoodTest, KnownSizeLarge) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StringPool pool;
  StandardRuleset ruleset;
  BoardState board_state = ruleset.CreateInitialBoardState(
      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(6, pool));

  ExpectBoardFood(board_state, 7);
  ExpectFoodAroundSnakes(board_state);
}

class StandardCreateNextBoardStateTest : public StandardRulesetTest {};

TEST_F(StandardCreateNextBoardStateTest, NoMoveFound) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  EXPECT_THROW(ruleset.CreateNextBoardState(initial_state, {}, 0),
               ErrorNoMoveFound);
}

TEST_F(StandardCreateNextBoardStateTest, ZeroLengthSnake) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body = {},
                  .health = 100,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  EXPECT_THROW(ruleset.CreateNextBoardState(initial_state,
                                            {{pool.Add("one"), Move::Down}}, 0),
               ErrorZeroLengthSnake);
}

TEST_F(StandardCreateNextBoardStateTest, MovesTail) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Down}}, 0);

  // Don't care about head in this test, only about the rest of the body.
  EXPECT_THAT(
      state.snakes,
      ElementsAre(SnakeBodyIs(ElementsAre(_, Point{1, 1}, Point{1, 2}))));
}

TEST_F(StandardCreateNextBoardStateTest, MovesHeadUp) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Up}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point{1, 2}, _, _))));
}

TEST_F(StandardCreateNextBoardStateTest, MovesHeadDown) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Down}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point{1, 0}, _, _))));
}

TEST_F(StandardCreateNextBoardStateTest, MovesHeadLeft) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point{0, 1}, _, _))));
}

TEST_F(StandardCreateNextBoardStateTest, MovesHeadRight) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Right}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point{2, 1}, _, _))));
}

TEST_F(StandardCreateNextBoardStateTest, MovesHeadUnknownContinue) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Unknown}}, 0);

  // Unknown move should move snake to its old direction.
  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point{1, 0}, _, _))));
}

TEST_F(StandardCreateNextBoardStateTest, MovesHeadUnknownUp) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 1},
                          Point{1, 1},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Unknown}}, 0);

  // Unknown move should move snake up if previous move is also unknown.
  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point{1, 2}, _, _))));
}

TEST_F(StandardCreateNextBoardStateTest, MovesTwoSnakes) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{3, 8},
                          Point{3, 7},
                          Point{3, 6},
                          Point{3, 5},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Left},
                                       {pool.Add("two"), Move::Right},
                                   },
                                   0);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAreArray({
          SnakeIs(pool.Add("one"),
                  ElementsAre(Point{0, 1}, Point{1, 1}, Point{1, 2})),
          SnakeIs(pool.Add("two"), ElementsAre(Point{4, 8}, Point{3, 8},
                                               Point{3, 7}, Point{3, 6})),
      }));
}

TEST_F(StandardCreateNextBoardStateTest, MoveReducesHealth) {
  StringPool pool;
  int initial_health = 75;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = initial_health,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Down}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Lt(initial_health))));
}

TEST_F(StandardCreateNextBoardStateTest, FoodGrowsSnake) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{0, 1},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 50,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point{0, 1}, Point{1, 1},
                                                  Point{1, 2}, Point{1, 2}))));
}

TEST_F(StandardCreateNextBoardStateTest, FoodRestoresHealth) {
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{0, 1},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = max_health / 2,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(max_health)));
}

TEST_F(StandardCreateNextBoardStateTest, DontEatFoodOtherPosition) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{10, 10},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 50,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeBodyIs(ElementsAre(
                                Point{0, 1}, Point{1, 1}, Point{1, 2}))));
}

TEST_F(StandardCreateNextBoardStateTest, EatenFoodDisappears) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{0, 1},
              Point{10, 10},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 50,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.food, ElementsAre(Point{10, 10}));
}

TEST_F(StandardCreateNextBoardStateTest, HeadToHeadFoodDisappears) {
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{1, 1},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 2},
                          Point{1, 3},
                          Point{1, 4},
                      },
                  .health = max_health / 2,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{2, 1},
                          Point{3, 1},
                          Point{4, 1},
                      },
                  .health = max_health / 2,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{
      .food_spawn_chance = 0,
      .minimum_food = 0,
  });
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   0);

  // Food must disappear.
  EXPECT_THAT(state.food, ElementsAre());
}

TEST_F(StandardCreateNextBoardStateTest, ZeroChanceNeverSpawnsFood) {
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food = {},
  };

  StandardRuleset ruleset(StandardRuleset::Config{
      .food_spawn_chance = 0,
      .minimum_food = 0,
  });

  for (int i = 0; i < 1000; ++i) {
    BoardState state = ruleset.CreateNextBoardState(initial_state, {}, 0);
    ASSERT_THAT(state.food.size(), Eq(0));
  }
}

TEST_F(StandardCreateNextBoardStateTest, HundredChanceAlwaysSpawnsFood) {
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food = {},
  };

  StandardRuleset ruleset(StandardRuleset::Config{
      .food_spawn_chance = 100,
      .minimum_food = 0,
  });

  for (int i = 0; i < 1000; ++i) {
    BoardState state = ruleset.CreateNextBoardState(initial_state, {}, 0);
    ASSERT_THAT(state.food.size(), Eq(1));
  }
}

TEST_F(StandardCreateNextBoardStateTest, SpawnFoodMinimum) {
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{1, 1},
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{
      .minimum_food = 7,
  });
  BoardState state = ruleset.CreateNextBoardState(initial_state, {}, 0);

  EXPECT_THAT(state.food.size(), Eq(7));
}

TEST_F(StandardCreateNextBoardStateTest, EatingOnLastMove) {
  // We want to specifically ensure that snakes eating food on their last turn
  // survive.
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{0, 1},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(max_health)));
}

TEST_F(StandardCreateNextBoardStateTest, IgnoresEliminatedSnakes) {
  // We want to specifically ensure that snakes eating food on their last turn
  // survive.
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{0, 1},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 10,
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::OutOfHealth},
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  // Snake has not moved, health has not changed.
  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs(
                  pool.Add("one"),
                  ElementsAre(Point{1, 1}, Point{1, 2}, Point{1, 3}), 10)));
  // Food hasn't disappeared.
  EXPECT_THAT(state.food, ElementsAre(Point{0, 1}));
}

class StandardEliminateSnakesTest : public StandardCreateNextBoardStateTest {};

TEST_F(StandardEliminateSnakesTest, OutOfHealth) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(pool.Add("one"), _, 0,
                                                EliminatedCause::OutOfHealth)));
}

TEST_F(StandardEliminateSnakesTest, OutOfBoundsUp) {
  StringPool pool;
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 0},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Up}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                                EliminatedCause::OutOfBounds)));
}

TEST_F(StandardEliminateSnakesTest, OutOfBoundsDown) {
  StringPool pool;
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 0},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Down}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                                EliminatedCause::OutOfBounds)));
}

TEST_F(StandardEliminateSnakesTest, OutOfBoundsLeft) {
  StringPool pool;
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 0},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                                EliminatedCause::OutOfBounds)));
}

TEST_F(StandardEliminateSnakesTest, OutOfBoundsRight) {
  StringPool pool;
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 0},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Right}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                                EliminatedCause::OutOfBounds)));
}

TEST_F(StandardEliminateSnakesTest, NoSelfCollision) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                  EliminatedCause::NotEliminated)));
}

TEST_F(StandardEliminateSnakesTest, NeckSelfCollision) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Up}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                  EliminatedCause::SelfCollision)));
}

TEST_F(StandardEliminateSnakesTest, RegularSelfCollision) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{2, 2},
                          Point{2, 1},
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                  EliminatedCause::SelfCollision)));
}

TEST_F(StandardEliminateSnakesTest, OwnTailChase) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{2, 2},
                          Point{2, 1},
                          Point{1, 1},
                          Point{1, 2},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Left}}, 0);

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                  EliminatedCause::NotEliminated)));
}

TEST_F(StandardEliminateSnakesTest, OtherNoCollision) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{2, 1},
                          Point{2, 2},
                          Point{2, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Right},
                                   },
                                   0);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), _, _, EliminatedCause::NotEliminated),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::NotEliminated)));
}

TEST_F(StandardEliminateSnakesTest, OtherBodyCollision) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{2, 1},
                          Point{2, 2},
                          Point{2, 3},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   0);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), _, _, EliminatedCause::NotEliminated),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::Collision,
                  pool.Add("one"))));
}

TEST_F(StandardEliminateSnakesTest, OtherTailChase) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 4},
                          Point{1, 5},
                          Point{1, 6},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Down},
                                   },
                                   0);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), _, _, EliminatedCause::NotEliminated),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::NotEliminated)));
}

TEST_F(StandardEliminateSnakesTest, HeadToHeadDifferentLength) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 3},
                          Point{1, 2},
                          Point{1, 1},
                          Point{1, 0},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 5},
                          Point{1, 6},
                          Point{1, 7},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Up},
                                       {pool.Add("two"), Move::Down},
                                   },
                                   0);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), _, _, EliminatedCause::NotEliminated),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::HeadToHeadCollision,
                  pool.Add("one"))));
}

TEST_F(StandardEliminateSnakesTest, HeadToHeadEqualLength) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 3},
                          Point{1, 2},
                          Point{1, 1},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 5},
                          Point{1, 6},
                          Point{1, 7},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Up},
                                       {pool.Add("two"), Move::Down},
                                   },
                                   0);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), _, _, EliminatedCause::HeadToHeadCollision,
                  pool.Add("two")),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::HeadToHeadCollision,
                  pool.Add("one"))));
}

TEST_F(StandardEliminateSnakesTest, PriorityOutOfHealthOutOfBounds) {
  StringPool pool;
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 0},
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Up}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                                EliminatedCause::OutOfHealth)));
}

TEST_F(StandardEliminateSnakesTest, PriorityOutOfHealthSelfCollision) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Up}}, 0);

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(pool.Add("one"), _, _,
                                                EliminatedCause::OutOfHealth)));
}

TEST_F(StandardEliminateSnakesTest, PriorityOutOfHealthOtherBodyCollision) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{2, 1},
                          Point{2, 2},
                          Point{2, 3},
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   0);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), _, _, EliminatedCause::NotEliminated),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::OutOfHealth,
                  Not(pool.Add("one")))));
}

TEST_F(StandardEliminateSnakesTest, PrioritySelfCollisionHeadToHead) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                          Point{1, 4},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{0, 0},
                          Point{1, 0},
                          Point{2, 0},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Right},
                                   },
                                   0);

  EXPECT_THAT(state.snakes,
              UnorderedElementsAre(
                  SnakeIs(pool.Add("one"), _, _, EliminatedCause::Collision,
                          pool.Add("two")),
                  SnakeIs(pool.Add("two"), _, _, EliminatedCause::SelfCollision,
                          Not(pool.Add("one")))));
}

TEST_F(StandardEliminateSnakesTest, PriorityOtherCollisionHeadToHead) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{0, 0},
                          Point{1, 0},
                          Point{2, 0},
                          Point{3, 0},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Right},
                                   },
                                   0);

  EXPECT_THAT(state.snakes,
              UnorderedElementsAre(
                  SnakeIs(pool.Add("one"), _, _, EliminatedCause::Collision,
                          pool.Add("two")),
                  SnakeIs(pool.Add("two"), _, _, EliminatedCause::SelfCollision,
                          Not(pool.Add("one")))));
}

TEST_F(StandardEliminateSnakesTest, OutOfHealthDoesntEliminateOthers) {
  // Snake Two can eliminate snake One for multiple reasons:
  // * One collided into Two's body.
  // * Two wins head-to-head.
  // But snake Two is out of health, so One doesn't get eliminated.
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{0, 0},
                          Point{1, 0},
                          Point{2, 0},
                          Point{3, 0},
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Right},
                                   },
                                   0);

  EXPECT_THAT(state.snakes,
              UnorderedElementsAre(
                  SnakeIs(pool.Add("one"), _, _, EliminatedCause::NotEliminated,
                          Not(pool.Add("two"))),
                  SnakeIs(pool.Add("two"), _, _, EliminatedCause::OutOfHealth,
                          Not(pool.Add("one")))));
}

TEST_F(StandardEliminateSnakesTest, OutOfBoundsDoesntEliminateOthers) {
  // Snake Two can eliminate snake One because One collided into Two's body.
  // But snake Two is out of bounds, so One doesn't get eliminated.
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{0, 0},
                          Point{1, 0},
                          Point{2, 0},
                          Point{3, 0},
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   0);

  EXPECT_THAT(state.snakes,
              UnorderedElementsAre(
                  SnakeIs(pool.Add("one"), _, _, EliminatedCause::NotEliminated,
                          Not(pool.Add("two"))),
                  SnakeIs(pool.Add("two"), _, _, EliminatedCause::OutOfBounds,
                          Not(pool.Add("one")))));
}

TEST_F(StandardCreateNextBoardStateTest, HeadToHeadFoodBothEliminated) {
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{1, 1},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 2},
                          Point{1, 3},
                          Point{1, 4},
                      },
                  .health = max_health / 2,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{2, 1},
                          Point{3, 1},
                          Point{4, 1},
                      },
                  .health = max_health / 2,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{
      .food_spawn_chance = 0,
      .minimum_food = 0,
  });
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   0);

  // Food must disappear.
  EXPECT_THAT(state.food, ElementsAre());
  // Both snakes eliminated.
  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), _, _, EliminatedCause::HeadToHeadCollision,
                  pool.Add("two")),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::HeadToHeadCollision,
                  pool.Add("one"))));
}

TEST_F(StandardCreateNextBoardStateTest, HeadToHeadFoodOneEliminated) {
  StringPool pool;
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{1, 1},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 2},
                          Point{1, 3},
                          Point{1, 4},
                          Point{1, 5},
                      },
                  .health = max_health / 2,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{2, 1},
                          Point{3, 1},
                          Point{4, 1},
                      },
                  .health = max_health / 2,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{
      .food_spawn_chance = 0,
      .minimum_food = 0,
  });
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   0);

  // Food must disappear.
  EXPECT_THAT(state.food, ElementsAre());
  // One snake survives and grows.
  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs(pool.Add("one"), ElementsAre(_, _, _, _, _), max_health,
                  EliminatedCause::NotEliminated, Not(pool.Add("two"))),
          SnakeIs(pool.Add("two"), _, _, EliminatedCause::HeadToHeadCollision,
                  pool.Add("one"))));
}

class StandardIsGameOverTest : public StandardRulesetTest {};

TEST_F(StandardIsGameOverTest, ZeroSnakes) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = {},
  };

  StandardRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

TEST_F(StandardIsGameOverTest, OneNotEliminatedSnake) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          },
  };

  StandardRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

TEST_F(StandardIsGameOverTest, OneEliminatedOneNotEliminatedSnake) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
              Snake{.eliminated_cause =
                        EliminatedCause{.cause = EliminatedCause::Collision}},
          },
  };

  StandardRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

TEST_F(StandardIsGameOverTest, TwoNotEliminatedSnake) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          },
  };

  StandardRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

TEST_F(StandardIsGameOverTest, OneOfFourEliminated) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
              Snake{.eliminated_cause =
                        EliminatedCause{.cause = EliminatedCause::OutOfBounds}},
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          },
  };

  StandardRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

TEST_F(StandardIsGameOverTest, ThreeOfFourEliminated) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{.eliminated_cause =
                        EliminatedCause{.cause = EliminatedCause::OutOfHealth}},
              Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::NotEliminated}},
              Snake{.eliminated_cause =
                        EliminatedCause{.cause = EliminatedCause::OutOfBounds}},
              Snake{.eliminated_cause =
                        EliminatedCause{
                            .cause = EliminatedCause::HeadToHeadCollision}},
          },
  };

  StandardRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
