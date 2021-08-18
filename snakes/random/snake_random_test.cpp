#include "snake_random.h"

#include <battlesnake/json/converter.h>
#include <unistd.h>

#include <fstream>
#include <streambuf>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::battlesnake::rules;
using namespace ::battlesnake::interface;

using ::testing::AnyOf;
using ::testing::Ne;

GameState LoadState(const std::string& test_name, StringPool& pool) {
  std::ifstream file_in("testdata/" + test_name + ".json");
  std::string str((std::istreambuf_iterator<char>(file_in)),
                  std::istreambuf_iterator<char>());
  return battlesnake::json::ParseJsonGameState(nlohmann::json::parse(str),
                                               pool);
}

TEST(BattlesnakeRandomTest, SnakeIsNotBoring) {
  SnakeRandom battlesnake;
  Customization customization = battlesnake.GetCustomization();

  // Anything but default!
  EXPECT_THAT(customization.color, Ne("#888888"));
  EXPECT_THAT(customization.head, Ne("default"));
  EXPECT_THAT(customization.tail, Ne("default"));
}

TEST(BattlesnakeRandomTest, SnakeMoves) {
  // Option 1: Construct board state manually. Don't forget to pool strings!
  StringPool pool;
  GameState state{
      .board{
          .width = kBoardSizeSmall,
          .height = kBoardSizeSmall,
          .food{
              Point{2, 2},
              Point{10, 7},
              Point{0, 0},
          },
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
              Snake{
                  .id = pool.Add("two"),
                  .body = SnakeBody::Create({
                      Point{5, 1},
                      Point{5, 2},
                      Point{5, 3},
                  }),
                  .health = 75,
              },
          }),
      },
  };
  state.you = state.board.snakes.front();

  SnakeRandom battlesnake;
  Battlesnake::MoveResponse move = battlesnake.Move(state);

  // Any reasonable move is OK.
  EXPECT_THAT(move.move, AnyOf(Move::Left, Move::Right, Move::Up, Move::Down));
}

TEST(BattlesnakeRandomTest, LoadTestFromJson) {
  // Option 2: Load test case from a json file.
  StringPool pool;
  GameState state = LoadState("test001", pool);

  SnakeRandom battlesnake;
  Battlesnake::MoveResponse move = battlesnake.Move(state);

  EXPECT_THAT(move.move, AnyOf(Move::Left, Move::Right, Move::Up, Move::Down));
}
