#include "standard_ruleset.h"

#include <algorithm>
#include <initializer_list>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ruleset_errors.h"

namespace battlesnake {
namespace engine {

namespace {

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Lt;

template <class T>
auto ValueBetween(const T& a, const T& b) {
  return AllOf(Ge(a), Lt(b));
}

class StandardRulesetTest : public testing::Test {};

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

  EXPECT_THAT(ruleset.IsGameOver(state), Eq(true));
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

class PlaceFoodTest : public StandardRulesetTest {
 protected:
  void ExpectBoardFood(const BoardState& state, int num_food) {
    EXPECT_THAT(state.food.size(), Eq(num_food));
    for (const Point& food : state.food) {
      EXPECT_THAT(food.x, ValueBetween(0, state.width));
      EXPECT_THAT(food.y, ValueBetween(0, state.height));
    }
  }

  std::vector<SnakeId> CreateSnakeIds(int n) {
    std::vector<SnakeId> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
      result.push_back("Snake" + std::to_string(n));
    }
    return result;
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
  ExpectBoardFood(ruleset.CreateInitialBoardState(
                      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(3)),
                  4);
}

TEST_F(PlaceFoodTest, KnownSizeMiddlle) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StandardRuleset ruleset;
  ExpectBoardFood(ruleset.CreateInitialBoardState(
                      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(8)),
                  9);
}

TEST_F(PlaceFoodTest, KnownSizeLarge) {
  // Food for each snake + 1 food in the middle for known board sizes.
  // Also tests known board size detection.
  StandardRuleset ruleset;
  ExpectBoardFood(ruleset.CreateInitialBoardState(
                      kBoardSizeSmall, kBoardSizeSmall, CreateSnakeIds(6)),
                  7);
}

}  // namespace

}  // namespace engine
}  // namespace battlesnake
