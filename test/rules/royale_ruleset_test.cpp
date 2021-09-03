#include "battlesnake/rules/royale_ruleset.h"

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
using ::testing::Lt;

template <class M>
auto SnakeHealthIs(M m) {
  return Field(&Snake::health, m);
}

class RoyaleRulesetTest : public testing::Test {
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

TEST_F(RoyaleRulesetTest, Sanity) {
  RoyaleRuleset ruleset;

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

class RoyaleCreateNextBoardStateTest : public RoyaleRulesetTest {};

TEST_F(RoyaleCreateNextBoardStateTest, NotInHazardDoesntAffectHealth) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{
              .id = pool.Add("one"),
              .body = SnakeBody::Create({
                  Point{1, 1},
                  Point{1, 2},
                  Point{1, 3},
              }),
              .health = 100,
          },
      }),
      .hazard_info =
          {
              .depth_left = 1,
              .depth_right = 0,
              .depth_top = 0,
              .depth_bottom = 0,
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  RoyaleRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(
      initial_state, SnakeMovesVector::Create({{pool.Add("one"), Move::Down}}),
      1, state);

  // Head is not in hazard, health must decrease by one.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Eq(99))));
}

TEST_F(RoyaleCreateNextBoardStateTest, InHazardReducesHealth) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{
              .id = pool.Add("one"),
              .body = SnakeBody::Create({
                  Point{0, 1},
                  Point{0, 2},
                  Point{0, 3},
              }),
              .health = 100,
          },
      }),
      .hazard_info =
          {
              .depth_left = 1,
              .depth_right = 0,
              .depth_top = 0,
              .depth_bottom = 0,
          },
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

TEST_F(RoyaleCreateNextBoardStateTest, FoodInHazardRestoresHealth) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .food =
          {
              Point{0, 0},
          },
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                      },
                  .health = 50,
              },
          },
      .hazards =
          {
              Point{0, 0},
              Point{0, 1},
              Point{0, 2},
              Point{0, 3},
              Point{0, 4},
              Point{0, 5},
              Point{0, 6},
              Point{0, 7},
              Point{0, 8},
              Point{0, 9},
              Point{0, 10},
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  RoyaleRuleset ruleset(StandardRuleset::Config{
      .food_spawn_chance = 0,
      .snake_max_health = 100,
  });
  BoardState state;
  ruleset.CreateNextBoardState(initial_state, {{pool.Add("one"), Move::Down}},
                               1, state);

  // Food must restore health to its max level.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Eq(100))));
}

TEST_F(RoyaleCreateNextBoardStateTest, MovesIntoHazard) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{
              .id = pool.Add("one"),
              .body = SnakeBody::Create({
                  Point{0, 1},
                  Point{0, 2},
                  Point{0, 3},
              }),
              .health = 100,
          },
      }),
      .hazard_info =
          {
              .depth_left = 0,
              .depth_right = 0,
              .depth_top = 0,
              .depth_bottom = 1,
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  RoyaleRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(
      initial_state, SnakeMovesVector::Create({{pool.Add("one"), Move::Down}}),
      1, state);

  // Move into hazard reduces health.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Lt(99))));
}

TEST_F(RoyaleCreateNextBoardStateTest, MovesOutOfHazard) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{
              .id = pool.Add("one"),
              .body = SnakeBody::Create({
                  Point{0, 1},
                  Point{0, 2},
                  Point{0, 3},
              }),
              .health = 100,
          },
      }),
      .hazard_info =
          {
              .depth_left = 1,
              .depth_right = 0,
              .depth_top = 0,
              .depth_bottom = 0,
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  RoyaleRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state{};
  ruleset.CreateNextBoardState(
      initial_state, SnakeMovesVector::Create({{pool.Add("one"), Move::Right}}),
      1, state);

  // Head is not in hazard after move, reduce health by just one.
  EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Eq(99))));
}

TEST_F(RoyaleCreateNextBoardStateTest, NewHazardDoesntAffectHealth) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = SnakesVector::Create({
          Snake{
              .id = pool.Add("one"),
              .body = SnakeBody::Create({
                  Point{1, 1},
                  Point{1, 2},
                  Point{1, 3},
              }),
              .health = 100,
          },
      }),
      .hazard_info =
          {
              .depth_left = 1,
              .depth_right = 0,
              .depth_top = 0,
              .depth_bottom = 0,
          },
  };

  // Disable spawning random food so that it doesn't interfere with tests.
  RoyaleRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0},
                        RoyaleRuleset::RoyaleConfig{.shrink_every_n_turns = 1});

  // Repeat test many times, because new hazard may appear on different sides.
  constexpr int attempts = 100;
  for (int i = 0; i < attempts; ++i) {
    BoardState state{};
    ruleset.CreateNextBoardState(
        initial_state,
        SnakeMovesVector::Create({{pool.Add("one"), Move::Down}}), 1, state);
    // Head is not in hazard, health must decrease by one.
    EXPECT_THAT(state.snakes, ElementsAre(SnakeHealthIs(Eq(99))));
  }
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
