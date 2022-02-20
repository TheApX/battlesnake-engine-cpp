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
using ::testing::UnorderedElementsAre;

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
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(
      initial_state, SnakeMovesVector::Create({{pool.Add("one"), Move::Down}}),
      1, state);

  // Head is in hazard, health must decrease by 15 points.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Eq(85))));
}

TEST_F(WrappedRulesetTest, NoHazardTurn2) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
  };

  int turn = 2;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(), UnorderedElementsAre());
}

TEST_F(WrappedRulesetTest, NoHazardTurn3) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{3, 4},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  int turn = 3;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(), UnorderedElementsAre(Point{3, 4}));
}

TEST_F(WrappedRulesetTest, NewHazardTurn5) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{3, 4},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  int turn = 5;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(), UnorderedElementsAre(Point{3, 4}, Point{3, 5}));
}

TEST_F(WrappedRulesetTest, NewHazardTurn8) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{3, 4},
              Point{3, 5},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  int turn = 8;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(),
              UnorderedElementsAre(Point{3, 4}, Point{3, 5}, Point{4, 5}));
}

TEST_F(WrappedRulesetTest, Turn5TopRight) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{10, 10},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  int turn = 5;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(), UnorderedElementsAre(Point{10, 10}));
}

TEST_F(WrappedRulesetTest, Turn17TopRight) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{10, 10},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  // This situation is ambiguous:
  // Option 1:
  //     XX
  // ----XX
  // ....|X
  // Option 2:
  //     XX
  //     XX
  // -----X
  // .....|

  int turn = 17;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(
      state.Hazard(),
      testing::AnyOf(UnorderedElementsAre(Point{10, 10}, Point{10, 9}),
                     UnorderedElementsAre(Point{10, 10}, Point{9, 10})));
}

TEST_F(WrappedRulesetTest, Turn17TopRight2) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{9, 10},
              Point{10, 10},
              Point{10, 9},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  // Explanation:
  //     XX
  // ----XX
  // ....+X
  // .....|

  int turn = 17;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(), UnorderedElementsAre(Point{9, 10}, Point{10, 10},
                                                   Point{10, 9}, Point{9, 9}));
}

TEST_F(WrappedRulesetTest, Turn17DownLeft) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{0, 1},
              Point{0, 2},
              Point{1, 0},
              Point{1, 1},
              Point{1, 2},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  // Explanation:
  // |....
  // XX...
  // XX...
  // +X---

  int turn = 17;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(),
              UnorderedElementsAre(Point{0, 1}, Point{0, 2}, Point{1, 0},
                                   Point{1, 1}, Point{1, 2}, Point{0, 0}));
}

TEST_F(WrappedRulesetTest, Turn17DownLeft2) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{0, 0},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  // Explanation:
  //  |..
  // XX--
  // XX
  // +X

  int turn = 17;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(), UnorderedElementsAre(Point{0, 0}));
}

TEST_F(WrappedRulesetTest, Turn20TopRight) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .hazard = CreateBoardBits(
          {
              Point{10, 10},
          },
          kBoardSizeMedium, kBoardSizeMedium),
  };

  // Explanation:
  //      XX
  //      XX
  // ----+XX
  // .....|

  int turn = 20;

  // Disable spawning random food so that it doesn't interfere with tests.
  WrappedRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(initial_state, SnakeMovesVector(), turn, state);

  EXPECT_THAT(state.Hazard(),
              UnorderedElementsAre(Point{10, 10}, Point{9, 10}));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
