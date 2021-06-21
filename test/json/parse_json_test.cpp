#include "battlesnake/json/converter.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace json {

namespace {

using ::testing::ElementsAreArray;
using ::testing::Eq;

using namespace ::battlesnake::engine;

class ParseJsonTest : public testing::Test {};

TEST_F(ParseJsonTest, PointSucceeds) {
  EXPECT_THAT(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"y":456})json")),
      Eq(Point(123, 456)));
}

TEST_F(ParseJsonTest, PointNoValue) {
  EXPECT_THROW(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"z":456})json")),
      ParseException);
}

TEST_F(ParseJsonTest, PointInvalidJsonType) {
  EXPECT_THROW(
      ParseJsonPoint(nlohmann::json::parse(R"json([{"x":123,"y":456}])json")),
      ParseException);
}

TEST_F(ParseJsonTest, PointInvalidValueType) {
  EXPECT_THROW(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"y":"456"})json")),
      ParseException);
}

TEST_F(ParseJsonTest, SnakeSucceeds) {
  auto json = nlohmann::json::parse(R"json(
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

  Snake expected_snake{
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

  Snake snake = ParseJsonSnake(json);

  EXPECT_THAT(snake.id, Eq(expected_snake.id));
  EXPECT_THAT(snake.body, ElementsAreArray(expected_snake.body));
  EXPECT_THAT(snake.health, Eq(expected_snake.health));
  EXPECT_THAT(snake.eliminated_cause.cause,
              Eq(expected_snake.eliminated_cause.cause));
  EXPECT_THAT(snake.eliminated_cause.by_id,
              Eq(expected_snake.eliminated_cause.by_id));
  EXPECT_THAT(snake.name, Eq(expected_snake.name));
  EXPECT_THAT(snake.latency, Eq(expected_snake.latency));
  EXPECT_THAT(snake.shout, Eq(expected_snake.shout));
  EXPECT_THAT(snake.squad, Eq(expected_snake.squad));
}

TEST_F(ParseJsonTest, SnakeNoOptional) {
  auto json = nlohmann::json::parse(R"json(
      {
          "id": "snake_id",
          "body": [
              {"x": 10, "y": 1},
              {"x": 10, "y": 2},
              {"x": 10, "y": 3}
          ],
          "length": 3,
          "head": {"x": 10, "y": 1},
          "health": 75
      }
  )json");

  Snake expected_snake{
      .id = "snake_id",
      .body =
          {
              Point(10, 1),
              Point(10, 2),
              Point(10, 3),
          },
      .health = 75,
      .latency = "0",
  };

  Snake snake = ParseJsonSnake(json);

  EXPECT_THAT(snake.id, Eq(expected_snake.id));
  EXPECT_THAT(snake.body, ElementsAreArray(expected_snake.body));
  EXPECT_THAT(snake.health, Eq(expected_snake.health));
  EXPECT_THAT(snake.eliminated_cause.cause,
              Eq(expected_snake.eliminated_cause.cause));
  EXPECT_THAT(snake.eliminated_cause.by_id,
              Eq(expected_snake.eliminated_cause.by_id));
  EXPECT_THAT(snake.name, Eq(expected_snake.name));
  EXPECT_THAT(snake.latency, Eq(expected_snake.latency));
  EXPECT_THAT(snake.shout, Eq(expected_snake.shout));
  EXPECT_THAT(snake.squad, Eq(expected_snake.squad));
}

TEST_F(ParseJsonTest, SnakeNoId) {
  auto json = nlohmann::json::parse(R"json(
      {
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

  EXPECT_THROW(ParseJsonSnake(json), ParseException);
}

TEST_F(ParseJsonTest, SnakeInvalidValueType) {
  auto json = nlohmann::json::parse(R"json(
      {
          "id": false,
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

  EXPECT_THROW(ParseJsonSnake(json), ParseException);
}

TEST_F(ParseJsonTest, SnakeInvalidJsonType) {
  auto json = nlohmann::json::parse(R"json(
      [{
          "id": false,
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
      }]
  )json");

  EXPECT_THROW(ParseJsonSnake(json), ParseException);
}

}  // namespace

}  // namespace json
}  // namespace battlesnake
