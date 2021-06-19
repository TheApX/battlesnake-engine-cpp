#include "standard_ruleset.h"

#include <algorithm>
#include <initializer_list>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ruleset_errors.h"

namespace battlesnake {
namespace engine {

namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;

class StandardRulesetTest : public testing::Test {
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

using CreateInitialBoardStateTest = StandardRulesetTest;

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

}  // namespace

}  // namespace engine
}  // namespace battlesnake
