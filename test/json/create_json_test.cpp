#include "battlesnake/json/converter.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace json {

namespace {

using ::testing::Eq;
using ::testing::IsNull;
using ::testing::Pointee;

using namespace ::battlesnake::rules;

class CreateJsonTest : public testing::Test {};

TEST_F(CreateJsonTest, Point) {
  nlohmann::json json = CreateJson(Point(123, 456));
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({"x":123,"y":456})json")));
}

TEST_F(CreateJsonTest, EliminatedSnake) {
  EXPECT_THAT(MaybeCreateJson(Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::OutOfHealth}}),
              IsNull());
}

TEST_F(CreateJsonTest, NotEliminatedSnake) {
  Snake snake{
      .id = "snake_id",
      .body =
          {
              Point(10, 1),
              Point(10, 2),
              Point(10, 3),
          },
      .health = 75,
      .name = "Test Caterpillar",
      .latency = "123",
      .shout = "Why are we shouting???",
      .squad = "The Suicide Squad",
  };

  auto expected_json = nlohmann::json::parse(R"json(
      {
          "id": "snake_id",
          "body": [
              {"x": 10, "y": 1},
              {"x": 10, "y": 2},
              {"x": 10, "y": 3}
          ],
          "length": 3,
          "head": {"x": 10, "y": 1},
          "health": 75,
          "name": "Test Caterpillar",
          "latency": "123",
          "shout": "Why are we shouting???",
          "squad": "The Suicide Squad"
      }
  )json");

  EXPECT_THAT(MaybeCreateJson(snake), Pointee(expected_json));
}

TEST_F(CreateJsonTest, BoardStateBasic) {
  BoardState state{
      .width = 5,
      .height = 15,
      .food = {},
      .snakes = {},
      .hazards = {},
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 5,
          "height": 15,
          "food": [],
          "snakes": [],
          "hazards": []
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateFood) {
  BoardState state{
      .width = 5,
      .height = 15,
      .food =
          {
              Point(0, 1),
              Point(4, 14),
          },
      .snakes = {},
      .hazards = {},
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 5,
          "height": 15,
          "food": [
              {"x": 0, "y": 1},
              {"x": 4, "y": 14}
          ],
          "snakes": [],
          "hazards": []
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateHazards) {
  BoardState state{
      .width = 5,
      .height = 15,
      .food = {},
      .snakes = {},
      .hazards =
          {
              Point(1, 0),
              Point(3, 10),
              Point(2, 5),
          },
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 5,
          "height": 15,
          "food": [],
          "snakes": [],
          "hazards": [
              {"x": 1, "y": 0},
              {"x": 3, "y": 10},
              {"x": 2, "y": 5}
          ]
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateSnakes) {
  BoardState state{
      .width = 5,
      .height = 15,
      .food = {},
      .snakes =
          {
              Snake{
                  .id = "snake_id",
                  .body =
                      {
                          Point(10, 1),
                          Point(10, 2),
                          Point(10, 3),
                      },
                  .health = 75,
                  .name = "Test Caterpillar",
                  .latency = "123",
                  .shout = "Why are we shouting???",
                  .squad = "The Suicide Squad",
              },
          },
      .hazards = {},
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 5,
          "height": 15,
          "food": [],
          "snakes": [{
              "id": "snake_id",
              "body": [
                  {"x": 10, "y": 1},
                  {"x": 10, "y": 2},
                  {"x": 10, "y": 3}
              ],
              "length": 3,
              "head": {"x": 10, "y": 1},
              "health": 75,
              "name": "Test Caterpillar",
              "latency": "123",
              "shout": "Why are we shouting???",
              "squad": "The Suicide Squad"
          }],
          "hazards": []
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateEliminatedSnake) {
  BoardState state{
      .width = 1,
      .height = 2,
      .food = {},
      .snakes =
          {
              Snake{
                  .id = "snake_id",
                  .body =
                      {
                          Point(10, 1),
                          Point(10, 2),
                          Point(10, 3),
                      },
                  .health = 75,
                  .eliminated_cause =
                      EliminatedCause{
                          .cause = EliminatedCause::HeadToHeadCollision,
                      },
                  .name = "Test Caterpillar",
                  .latency = "123",
                  .shout = "Why are we shouting???",
                  .squad = "The Suicide Squad",
              },
          },
      .hazards = {},
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 1,
          "height": 2,
          "food": [],
          "snakes": [],
          "hazards": []
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

}  // namespace

}  // namespace json
}  // namespace battlesnake
