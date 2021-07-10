#include "battlesnake/rules/constrictor_ruleset.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace rules {

namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Field;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Lt;

template <class M>
auto SnakeHealthIs(M m) {
  return Field(&Snake::health, m);
}

template <class M>
auto SnakeBodyIs(const M& m) {
  return Field(&Snake::body, m);
}

class ConstrictorRulesetTest : public testing::Test {
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

TEST_F(ConstrictorRulesetTest, Sanity) {
  ConstrictorRuleset ruleset;

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

TEST_F(ConstrictorRulesetTest, NoFoodInitially) {
  ConstrictorRuleset ruleset;

  StringPool pool;
  BoardState state = ruleset.CreateInitialBoardState(11, 11,
                                                     {
                                                         pool.Add("snake1"),
                                                         pool.Add("snake2"),
                                                     });

  EXPECT_THAT(state.food, ElementsAre());
}

class ConstrictorCreateNextBoardStateTest : public ConstrictorRulesetTest {};

TEST_F(ConstrictorCreateNextBoardStateTest, KeepsHealth) {
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
  ConstrictorRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Down}}, 1);

  // Health shouldn't decrease.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Eq(100))));
}

TEST_F(ConstrictorCreateNextBoardStateTest, GrowsSnake) {
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
  ConstrictorRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Down}}, 1);

  // Body should grow.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeBodyIs(ElementsAreArray({
                                Point{1, 0},
                                Point{1, 1},
                                Point{1, 2},
                                Point{1, 2},
                            }))));
}

TEST_F(ConstrictorCreateNextBoardStateTest, DoesnGrowInitialSnake) {
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

  // Disable spawning random food so that it doesn't interfere with tests.
  ConstrictorRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state = ruleset.CreateNextBoardState(
      initial_state, {{pool.Add("one"), Move::Down}}, 1);

  // Body shouldn't grow.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeBodyIs(ElementsAreArray({
                                Point{1, 0},
                                Point{1, 1},
                                Point{1, 1},
                            }))));
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
