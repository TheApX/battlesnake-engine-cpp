#include "battlesnake/rules/wrapped_ruleset.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace rules {

namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Field;
using ::testing::IsFalse;
using ::testing::IsTrue;

template <class M>
auto SnakeHealthIs(M m) {
  return Field(&Snake::health, m);
}

class WrappedRulesetTest : public testing::Test {
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

TEST_F(WrappedRulesetTest, Sanity) {
  WrappedRuleset ruleset;

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

TEST_F(WrappedRulesetTest, IsWrapped) {
  WrappedRuleset ruleset;

  EXPECT_THAT(ruleset.IsWrapped(), IsTrue());
}

TEST_F(WrappedRulesetTest, InHazardReducesHealth) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{
              .id = pool.Add("one"),
              .body = SnakeBody::Create({
                  Point{3, 3},
                  Point{3, 3},
                  Point{3, 3},
              }),
              .health = 100,
          },
      }),
      .hazard = CreateBoardBits(
          {
              Point{2, 2},
              Point{3, 2},
              Point{4, 2},
              Point{2, 3},
              Point{3, 3},
              Point{4, 3},
              Point{2, 4},
              Point{3, 4},
              Point{4, 4},
          },
          kBoardSizeSmall, kBoardSizeSmall),
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  RoyaleRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(
      initial_state, SnakeMovesVector::Create({{pool.Add("one"), Move::Down}}),
      1, state);

  // Head is in hazard, health must decrease by 15 points.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Eq(85))));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
