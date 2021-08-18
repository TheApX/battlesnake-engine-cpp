#include "battlesnake/rules/solo_ruleset.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace rules {

namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;

class SoloRulesetTest : public testing::Test {
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

TEST_F(SoloRulesetTest, Sanity) {
  SoloRuleset ruleset;

  BoardState state = ruleset.CreateInitialBoardState(0, 0, {});
  EXPECT_THAT(state.width, Eq(0));
  EXPECT_THAT(state.height, Eq(0));
  EXPECT_THAT(state.snakes, ElementsAre());

  BoardState new_state{};
  ruleset.CreateNextBoardState(state, {}, 0, new_state);
  EXPECT_THAT(new_state.width, Eq(0));
  EXPECT_THAT(new_state.height, Eq(0));
  EXPECT_THAT(new_state.snakes, ElementsAre());

  EXPECT_THAT(ruleset.IsGameOver(new_state), IsTrue());
}

class SoloIsGameOverTest : public SoloRulesetTest {};

TEST_F(SoloIsGameOverTest, ZeroSnakes) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = {},
  };

  SoloRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

TEST_F(SoloIsGameOverTest, OneNotEliminatedSnake) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
      }),
  };

  SoloRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

TEST_F(SoloIsGameOverTest, OneEliminatedOneNotEliminatedSnake) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::Collision}},
      }),
  };

  SoloRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

TEST_F(SoloIsGameOverTest, TwoNotEliminatedSnake) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
      }),
  };

  SoloRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

TEST_F(SoloIsGameOverTest, OneOfFourEliminated) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::OutOfBounds}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
      }),
  };

  SoloRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

TEST_F(SoloIsGameOverTest, ThreeOfFourEliminated) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::OutOfHealth}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::NotEliminated}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::OutOfBounds}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause =
                                        EliminatedCause::HeadToHeadCollision}},
      }),
  };

  SoloRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

TEST_F(SoloIsGameOverTest, FourOfFourEliminated) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::OutOfHealth}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::Collision}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause = EliminatedCause::OutOfBounds}},
          Snake{.eliminated_cause =
                    EliminatedCause{.cause =
                                        EliminatedCause::HeadToHeadCollision}},
      }),
  };

  SoloRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
