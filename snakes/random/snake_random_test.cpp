#include "snake_random.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::battlesnake::rules;
using namespace ::battlesnake::interface;

using ::testing::AnyOf;
using ::testing::Ne;

TEST(BattlesnakeRandomTest, SnakeIsNotBoring) {
  SnakeRandom battlesnake;

  Customization customization = battlesnake.GetCustomization();

  // Anything but default!
  EXPECT_THAT(customization.color, Ne("#888888"));
  EXPECT_THAT(customization.head, Ne("default"));
  EXPECT_THAT(customization.tail, Ne("default"));
}

TEST(BattlesnakeRandomTest, SnakeMoves) {
  SnakeRandom battlesnake;

  GameState state{
      // The snake doesn't care about game state, so don't set anything.
  };

  Battlesnake::MoveResponse move = battlesnake.Move(state);

  // Any reasonable move is OK.
  EXPECT_THAT(move.move, AnyOf(Move::Left, Move::Right, Move::Up, Move::Down));
}
