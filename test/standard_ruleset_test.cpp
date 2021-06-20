#include "standard_ruleset.h"

#include <algorithm>
#include <initializer_list>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ruleset_errors.h"

namespace battlesnake {
namespace engine {

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
  std::vector<SnakeId> CreateSnakeIds(int n) {
    std::vector<SnakeId> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
      result.push_back("Snake" + std::to_string(n));
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

  BoardState new_state = ruleset.CreateNextBoardState(state, {});
  EXPECT_THAT(state.width, Eq(0));
  EXPECT_THAT(state.height, Eq(0));
  EXPECT_THAT(state.snakes, ElementsAre());

  EXPECT_THAT(ruleset.IsGameOver(state), IsTrue());
}

class CreateInitialBoardStateTest : public StandardRulesetTest {
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

TEST_F(CreateInitialBoardStateTest, Small1by1) {
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(1, 1, {"one"}), 1, 1, 0, {"one"});
}

TEST_F(CreateInitialBoardStateTest, Small1by2) {
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(1, 2, {"one"}), 1, 2, 0, {"one"});
}

TEST_F(CreateInitialBoardStateTest, Small1by4) {
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(1, 4, {"one"}), 1, 4, 1, {"one"});
}

TEST_F(CreateInitialBoardStateTest, Small2by2) {
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(2, 2, {"one"}), 2, 2, 1, {"one"});
}

TEST_F(CreateInitialBoardStateTest, NonStandardSize) {
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(9, 8, {"one"}), 9, 8, 1, {"one"});
}

TEST_F(CreateInitialBoardStateTest, SmallTwoSnakes) {
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(2, 2, {"one", "two"}), 2, 2, 0,
              {"one", "two"});
}

TEST_F(CreateInitialBoardStateTest, NoRoom1by1) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(1, 1, {"one", "two"}),
               ErrorNoRoomForSnake);
}

TEST_F(CreateInitialBoardStateTest, NoRoom1by2) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(1, 2, {"one", "two"}),
               ErrorNoRoomForSnake);
}

TEST_F(CreateInitialBoardStateTest, SmallBoard) {
  StandardRuleset ruleset;
  ExpectBoard(ruleset.CreateInitialBoardState(kBoardSizeSmall, kBoardSizeSmall,
                                              {"one", "two"}),
              kBoardSizeSmall, kBoardSizeSmall, 3, {"one", "two"});
}

class PlaceSnakeTest : public StandardRulesetTest {
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

TEST_F(PlaceSnakeTest, Small1by1) {
  StandardRuleset ruleset;
  ExpectBoardSnakes(ruleset.CreateInitialBoardState(1, 1, CreateSnakeIds(1)),
                    1);
}

TEST_F(PlaceSnakeTest, Small1by1TwoSnakes) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(1, 1, CreateSnakeIds(2)),
               ErrorNoRoomForSnake);
}

TEST_F(PlaceSnakeTest, Small1by2TwoSnakes) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(1, 2, CreateSnakeIds(2)),
               ErrorNoRoomForSnake);
}

TEST_F(PlaceSnakeTest, Small2by1TwoSnakes) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(2, 1, CreateSnakeIds(2)),
               ErrorNoRoomForSnake);
}

TEST_F(PlaceSnakeTest, Small2by2TwoSnakes) {
  StandardRuleset ruleset;
  ExpectBoardSnakes(ruleset.CreateInitialBoardState(2, 2, CreateSnakeIds(2)),
                    2);
}

TEST_F(PlaceSnakeTest, EnoughSpaceForManySnakes) {
  StandardRuleset ruleset;
  ExpectBoardSnakes(ruleset.CreateInitialBoardState(5, 10, CreateSnakeIds(25)),
                    25);
}

TEST_F(PlaceSnakeTest, NotEnoughSpaceForManySnakes) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(5, 10, CreateSnakeIds(26)),
               ErrorNoRoomForSnake);
}

TEST_F(PlaceSnakeTest, KnownSizeSmallOneSnake) {
  StandardRuleset ruleset;
  ExpectBoardSnakes(ruleset.CreateInitialBoardState(
                        kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(1)),
                    1);
}

TEST_F(PlaceSnakeTest, KnownSizeSmallMaxSnakes) {
  StandardRuleset ruleset;
  ExpectBoardSnakes(ruleset.CreateInitialBoardState(
                        kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(8)),
                    8);
}

TEST_F(PlaceSnakeTest, KnownSizeSmallTooManySnakes) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(kBoardSizeSmall, kBoardSizeSmall,
                                               CreateSnakeIds(9)),
               ErrorTooManySnakes);
}

TEST_F(PlaceSnakeTest, KnownSizeMediumMaxSnakes) {
  StandardRuleset ruleset;
  ExpectBoardSnakes(ruleset.CreateInitialBoardState(
                        kBoardSizeMedium, kBoardSizeMedium, CreateSnakeIds(8)),
                    8);
}

TEST_F(PlaceSnakeTest, KnownSizeMediumTooManySnakes) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(
                   kBoardSizeMedium, kBoardSizeMedium, CreateSnakeIds(9)),
               ErrorTooManySnakes);
}

TEST_F(PlaceSnakeTest, KnownSizeLargeMaxSnakes) {
  StandardRuleset ruleset;
  ExpectBoardSnakes(ruleset.CreateInitialBoardState(
                        kBoardSizeLarge, kBoardSizeLarge, CreateSnakeIds(8)),
                    8);
}

TEST_F(PlaceSnakeTest, KnownSizeLargeTooManySnakes) {
  StandardRuleset ruleset;
  EXPECT_THROW(ruleset.CreateInitialBoardState(kBoardSizeLarge, kBoardSizeLarge,
                                               CreateSnakeIds(9)),
               ErrorTooManySnakes);
}

class PlaceFoodTest : public StandardRulesetTest {
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
          Point(head.x - 1, head.y - 1),
          Point(head.x - 1, head.y + 1),
          Point(head.x + 1, head.y - 1),
          Point(head.x + 1, head.y + 1),
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

TEST_F(PlaceFoodTest, Small1by1) {
  // The only cell is taken by snake, no place for food.
  StandardRuleset ruleset;
  ExpectBoardFood(ruleset.CreateInitialBoardState(1, 1, CreateSnakeIds(1)), 0);
}

TEST_F(PlaceFoodTest, Small1by2) {
  // One cell is taken by snake, but the other one is not even.
  StandardRuleset ruleset;
  ExpectBoardFood(ruleset.CreateInitialBoardState(1, 2, CreateSnakeIds(1)), 0);
}

TEST_F(PlaceFoodTest, ManySnakesMuchSpace) {
  // Many randomly placed snakes, food for everybody.
  StandardRuleset ruleset;
  ExpectBoardFood(ruleset.CreateInitialBoardState(101, 202, CreateSnakeIds(17)),
                  17);
}

TEST_F(PlaceFoodTest, AllFreeSpaceFilledIn) {
  // Many randomly placed snakes, space for some food, but not for everybody.
  StandardRuleset ruleset;
  ExpectBoardFood(ruleset.CreateInitialBoardState(10, 20, CreateSnakeIds(60)),
                  40);
}

TEST_F(PlaceFoodTest, KnownSizeSmall) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StandardRuleset ruleset;
  BoardState board_state = ruleset.CreateInitialBoardState(
      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(3));

  ExpectBoardFood(board_state, 4);
  ExpectFoodAroundSnakes(board_state);
}

TEST_F(PlaceFoodTest, KnownSizeMiddlle) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StandardRuleset ruleset;
  BoardState board_state = ruleset.CreateInitialBoardState(
      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(8));

  ExpectBoardFood(board_state, 9);
  ExpectFoodAroundSnakes(board_state);
}

TEST_F(PlaceFoodTest, KnownSizeLarge) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StandardRuleset ruleset;
  BoardState board_state = ruleset.CreateInitialBoardState(
      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(6));

  ExpectBoardFood(board_state, 7);
  ExpectFoodAroundSnakes(board_state);
}

class TestCreateNextBoardState : public StandardRulesetTest {};

TEST_F(TestCreateNextBoardState, NoMoveFound) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  EXPECT_THROW(ruleset.CreateNextBoardState(initial_state, {}),
               ErrorNoMoveFound);
}

TEST_F(TestCreateNextBoardState, ZeroLengthSnake) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body = {},
                  .health = 100,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  EXPECT_THROW(
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Down}}),
      ErrorZeroLengthSnake);
}

TEST_F(TestCreateNextBoardState, MovesTail) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Down}});

  // Don't care about head in this test, only about the rest of the body.
  EXPECT_THAT(
      state.snakes,
      ElementsAre(SnakeBodyIs(ElementsAre(_, Point(1, 1), Point(1, 2)))));
}

TEST_F(TestCreateNextBoardState, MovesHeadUp) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Up}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point(1, 2), _, _))));
}

TEST_F(TestCreateNextBoardState, MovesHeadDown) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Down}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point(1, 0), _, _))));
}

TEST_F(TestCreateNextBoardState, MovesHeadLeft) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point(0, 1), _, _))));
}

TEST_F(TestCreateNextBoardState, MovesHeadRight) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Right}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point(2, 1), _, _))));
}

TEST_F(TestCreateNextBoardState, MovesHeadUnknownContinue) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Unknown}});

  // Unknown move should move snake to its old direction.
  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point(1, 0), _, _))));
}

TEST_F(TestCreateNextBoardState, MovesHeadUnknownUp) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 1),
                          Point(1, 1),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Unknown}});

  // Unknown move should move snake up if previous move is also unknown.
  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point(1, 2), _, _))));
}

TEST_F(TestCreateNextBoardState, MovesTwoSnakes) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(3, 8),
                          Point(3, 7),
                          Point(3, 6),
                          Point(3, 5),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Left},
                                                      {"two", Move::Right},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAreArray({
          SnakeIs("one", ElementsAre(Point(0, 1), Point(1, 1), Point(1, 2))),
          SnakeIs("two", ElementsAre(Point(4, 8), Point(3, 8), Point(3, 7),
                                     Point(3, 6))),
      }));
}

TEST_F(TestCreateNextBoardState, MoveReducesHealth) {
  int initial_health = 75;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = initial_health,
              },
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Down}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Lt(initial_health))));
}

TEST_F(TestCreateNextBoardState, FoodGrowsSnake) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(0, 1),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 50,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeBodyIs(ElementsAre(Point(0, 1), Point(1, 1),
                                                  Point(1, 2), Point(1, 2)))));
}

TEST_F(TestCreateNextBoardState, FoodRestoresHealth) {
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(0, 1),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = max_health / 2,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(max_health)));
}

TEST_F(TestCreateNextBoardState, DontEatFoodOtherPosition) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(10, 10),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 50,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeBodyIs(ElementsAre(
                                Point(0, 1), Point(1, 1), Point(1, 2)))));
}

TEST_F(TestCreateNextBoardState, EatenFoodDisappears) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(0, 1),
              Point(10, 10),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 50,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.food, ElementsAre(Point(10, 10)));
}

TEST_F(TestCreateNextBoardState, HeadToHeadFoodDisappears) {
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(1, 1),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 2),
                          Point(1, 3),
                          Point(1, 4),
                      },
                  .health = max_health / 2,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(2, 1),
                          Point(3, 1),
                          Point(4, 1),
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
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Left},
                                                  });

  // Food must disappear.
  EXPECT_THAT(state.food, ElementsAre());
}

TEST_F(TestCreateNextBoardState, ZeroChanceNeverSpawnsFood) {
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
    BoardState state = ruleset.CreateNextBoardState(initial_state, {});
    ASSERT_THAT(state.food.size(), Eq(0));
  }
}

TEST_F(TestCreateNextBoardState, HundredChanceAlwaysSpawnsFood) {
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
    BoardState state = ruleset.CreateNextBoardState(initial_state, {});
    ASSERT_THAT(state.food.size(), Eq(1));
  }
}

TEST_F(TestCreateNextBoardState, SpawnFoodMinimum) {
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(1, 1),
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{
      .minimum_food = 7,
  });
  BoardState state = ruleset.CreateNextBoardState(initial_state, {});

  EXPECT_THAT(state.food.size(), Eq(7));
}

TEST_F(TestCreateNextBoardState, EatingOnLastMove) {
  // We want to specifically ensure that snakes eating food on their last turn
  // survive.
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(0, 1),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(max_health)));
}

TEST_F(TestCreateNextBoardState, IgnoresEliminatedSnakes) {
  // We want to specifically ensure that snakes eating food on their last turn
  // survive.
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(0, 1),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 10,
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::OutOfHealth},
              },
          },
  };

  StandardRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  // Snake has not moved, health has not changed.
  EXPECT_THAT(
      state.snakes,
      ElementsAre(SnakeIs(
          "one", ElementsAre(Point(1, 1), Point(1, 2), Point(1, 3)), 10)));
  // Food hasn't disappeared.
  EXPECT_THAT(state.food, ElementsAre(Point(0, 1)));
}

class TestEliminateSnakes : public TestCreateNextBoardState {};

TEST_F(TestEliminateSnakes, OutOfHealth) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs("one", _, 0, EliminatedCause::OutOfHealth)));
}

TEST_F(TestEliminateSnakes, OutOfBoundsUp) {
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(0, 0),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Up}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs("one", _, _, EliminatedCause::OutOfBounds)));
}

TEST_F(TestEliminateSnakes, OutOfBoundsDown) {
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(0, 0),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Down}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs("one", _, _, EliminatedCause::OutOfBounds)));
}

TEST_F(TestEliminateSnakes, OutOfBoundsLeft) {
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(0, 0),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs("one", _, _, EliminatedCause::OutOfBounds)));
}

TEST_F(TestEliminateSnakes, OutOfBoundsRight) {
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(0, 0),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Right}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs("one", _, _, EliminatedCause::OutOfBounds)));
}

TEST_F(TestEliminateSnakes, NoSelfCollision) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(
                                "one", _, _, EliminatedCause::NotEliminated)));
}

TEST_F(TestEliminateSnakes, NeckSelfCollision) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Up}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(
                                "one", _, _, EliminatedCause::SelfCollision)));
}

TEST_F(TestEliminateSnakes, RegularSelfCollision) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(2, 2),
                          Point(2, 1),
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(
                                "one", _, _, EliminatedCause::SelfCollision)));
}

TEST_F(TestEliminateSnakes, OwnTailChase) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(2, 2),
                          Point(2, 1),
                          Point(1, 1),
                          Point(1, 2),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Left}});

  EXPECT_THAT(state.snakes, ElementsAre(SnakeIs(
                                "one", _, _, EliminatedCause::NotEliminated)));
}

TEST_F(TestEliminateSnakes, OtherNoCollision) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(2, 1),
                          Point(2, 2),
                          Point(2, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Right},
                                                  });

  EXPECT_THAT(state.snakes,
              UnorderedElementsAre(
                  SnakeIs("one", _, _, EliminatedCause::NotEliminated),
                  SnakeIs("two", _, _, EliminatedCause::NotEliminated)));
}

TEST_F(TestEliminateSnakes, OtherBodyCollision) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(2, 1),
                          Point(2, 2),
                          Point(2, 3),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Left},
                                                  });

  EXPECT_THAT(state.snakes,
              UnorderedElementsAre(
                  SnakeIs("one", _, _, EliminatedCause::NotEliminated),
                  SnakeIs("two", _, _, EliminatedCause::Collision, "one")));
}

TEST_F(TestEliminateSnakes, OtherTailChase) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(1, 4),
                          Point(1, 5),
                          Point(1, 6),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Down},
                                                  });

  EXPECT_THAT(state.snakes,
              UnorderedElementsAre(
                  SnakeIs("one", _, _, EliminatedCause::NotEliminated),
                  SnakeIs("two", _, _, EliminatedCause::NotEliminated)));
}

TEST_F(TestEliminateSnakes, HeadToHeadDifferentLength) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 3),
                          Point(1, 2),
                          Point(1, 1),
                          Point(1, 0),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(1, 5),
                          Point(1, 6),
                          Point(1, 7),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Up},
                                                      {"two", Move::Down},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::NotEliminated),
          SnakeIs("two", _, _, EliminatedCause::HeadToHeadCollision, "one")));
}

TEST_F(TestEliminateSnakes, HeadToHeadEqualLength) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 3),
                          Point(1, 2),
                          Point(1, 1),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(1, 5),
                          Point(1, 6),
                          Point(1, 7),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Up},
                                                      {"two", Move::Down},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::HeadToHeadCollision, "two"),
          SnakeIs("two", _, _, EliminatedCause::HeadToHeadCollision, "one")));
}

TEST_F(TestEliminateSnakes, PriorityOutOfHealthOutOfBounds) {
  BoardState initial_state{
      .width = 1,
      .height = 1,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(0, 0),
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Up}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs("one", _, _, EliminatedCause::OutOfHealth)));
}

TEST_F(TestEliminateSnakes, PriorityOutOfHealthSelfCollision) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {{"one", Move::Up}});

  EXPECT_THAT(state.snakes,
              ElementsAre(SnakeIs("one", _, _, EliminatedCause::OutOfHealth)));
}

TEST_F(TestEliminateSnakes, PriorityOutOfHealthOtherBodyCollision) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(2, 1),
                          Point(2, 2),
                          Point(2, 3),
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Left},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::NotEliminated),
          SnakeIs("two", _, _, EliminatedCause::OutOfHealth, Not("one"))));
}

TEST_F(TestEliminateSnakes, PrioritySelfCollisionHeadToHead) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                          Point(1, 4),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(0, 0),
                          Point(1, 0),
                          Point(2, 0),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Right},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::Collision, "two"),
          SnakeIs("two", _, _, EliminatedCause::SelfCollision, Not("one"))));
}

TEST_F(TestEliminateSnakes, PriorityOtherCollisionHeadToHead) {
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(0, 0),
                          Point(1, 0),
                          Point(2, 0),
                          Point(3, 0),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Right},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::Collision, "two"),
          SnakeIs("two", _, _, EliminatedCause::SelfCollision, Not("one"))));
}

TEST_F(TestEliminateSnakes, OutOfHealthDoesntEliminateOthers) {
  // Snake Two can eliminate snake One for multiple reasons:
  // * One collided into Two's body.
  // * Two wins head-to-head.
  // But snake Two is out of health, so One doesn't get eliminated.
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(0, 0),
                          Point(1, 0),
                          Point(2, 0),
                          Point(3, 0),
                      },
                  .health = 1,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Right},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::NotEliminated, Not("two")),
          SnakeIs("two", _, _, EliminatedCause::OutOfHealth, Not("one"))));
}

TEST_F(TestEliminateSnakes, OutOfBoundsDoesntEliminateOthers) {
  // Snake Two can eliminate snake One because One collided into Two's body.
  // But snake Two is out of bounds, so One doesn't get eliminated.
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 1),
                          Point(1, 2),
                          Point(1, 3),
                      },
                  .health = 100,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(0, 0),
                          Point(1, 0),
                          Point(2, 0),
                          Point(3, 0),
                      },
                  .health = 100,
              },
          },
  };

  StandardRuleset ruleset;
  BoardState state =
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Left},
                                                  });

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::NotEliminated, Not("two")),
          SnakeIs("two", _, _, EliminatedCause::OutOfBounds, Not("one"))));
}

TEST_F(TestCreateNextBoardState, HeadToHeadFoodBothEliminated) {
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(1, 1),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 2),
                          Point(1, 3),
                          Point(1, 4),
                      },
                  .health = max_health / 2,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(2, 1),
                          Point(3, 1),
                          Point(4, 1),
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
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Left},
                                                  });

  // Food must disappear.
  EXPECT_THAT(state.food, ElementsAre());
  // Both snakes eliminated.
  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", _, _, EliminatedCause::HeadToHeadCollision, "two"),
          SnakeIs("two", _, _, EliminatedCause::HeadToHeadCollision, "one")));
}

TEST_F(TestCreateNextBoardState, HeadToHeadFoodOneEliminated) {
  int max_health = StandardRuleset::Config::Default().snake_max_health;

  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point(1, 1),
          },
      .snakes =
          {
              Snake{
                  .id = "one",
                  .body =
                      {
                          Point(1, 2),
                          Point(1, 3),
                          Point(1, 4),
                          Point(1, 5),
                      },
                  .health = max_health / 2,
              },
              Snake{
                  .id = "two",
                  .body =
                      {
                          Point(2, 1),
                          Point(3, 1),
                          Point(4, 1),
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
      ruleset.CreateNextBoardState(initial_state, {
                                                      {"one", Move::Down},
                                                      {"two", Move::Left},
                                                  });

  // Food must disappear.
  EXPECT_THAT(state.food, ElementsAre());
  // One snake survives and grows.
  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(
          SnakeIs("one", ElementsAre(_, _, _, _, _), max_health,
                  EliminatedCause::NotEliminated, Not("two")),
          SnakeIs("two", _, _, EliminatedCause::HeadToHeadCollision, "one")));
}

class IsGameOverTest : public StandardRulesetTest {};

TEST_F(IsGameOverTest, ZeroSnakes) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = {},
  };

  StandardRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

TEST_F(IsGameOverTest, OneNotEliminatedSnake) {
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

TEST_F(IsGameOverTest, OneEliminatedOneNotEliminatedSnake) {
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

TEST_F(IsGameOverTest, TwoNotEliminatedSnake) {
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

TEST_F(IsGameOverTest, OneOfFourEliminated) {
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

TEST_F(IsGameOverTest, ThreeOfFourEliminated) {
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

}  // namespace engine
}  // namespace battlesnake
