#include "battlesnake/json/converter.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace json {

namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Field;

using namespace ::battlesnake::rules;

class ParseJsonTest : public testing::Test {};

TEST_F(ParseJsonTest, PointSucceeds) {
  EXPECT_THAT(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"y":456})json")),
      Eq(Point{123, 456}));
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
              Point{10, 1},
              Point{10, 2},
              Point{10, 3},
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
              Point{10, 1},
              Point{10, 2},
              Point{10, 3},
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

TEST_F(ParseJsonTest, BoardStateBasic) {
  auto json = nlohmann::json::parse(R"json(
        {
          "width": 5,
          "height": 15,
          "food": [],
          "snakes": [],
          "hazards": []
        }
    )json");

  BoardState expected_state{
      .width = 5,
      .height = 15,
      .food = {},
      .hazards = {},
  };

  BoardState state = ParseJsonBoard(json);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.food, ElementsAreArray(expected_state.food));
  EXPECT_THAT(state.snakes, ElementsAre());
  EXPECT_THAT(state.hazards, ElementsAreArray(expected_state.hazards));
}

TEST_F(ParseJsonTest, BoardStateFood) {
  auto json = nlohmann::json::parse(R"json(
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

  BoardState expected_state{
      .width = 5,
      .height = 15,
      .food =
          {
              Point{0, 1},
              Point{4, 14},
          },
      .hazards = {},
  };

  BoardState state = ParseJsonBoard(json);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.food, ElementsAreArray(expected_state.food));
  EXPECT_THAT(state.snakes, ElementsAre());
  EXPECT_THAT(state.hazards, ElementsAreArray(expected_state.hazards));
}

TEST_F(ParseJsonTest, BoardStateHazards) {
  auto json = nlohmann::json::parse(R"json(
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

  BoardState expected_state{
      .width = 5,
      .height = 15,
      .food = {},
      .hazards =
          {
              Point{1, 0},
              Point{3, 10},
              Point{2, 5},
          },
  };

  BoardState state = ParseJsonBoard(json);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.food, ElementsAreArray(expected_state.food));
  EXPECT_THAT(state.snakes, ElementsAre());
  EXPECT_THAT(state.hazards, ElementsAreArray(expected_state.hazards));
}

TEST_F(ParseJsonTest, BoardStateSnakes) {
  auto json = nlohmann::json::parse(R"json(
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

  BoardState expected_state{
      .width = 5,
      .height = 15,
      .food = {},
      .hazards = {},
  };

  BoardState state = ParseJsonBoard(json);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.food, ElementsAreArray(expected_state.food));
  EXPECT_THAT(state.snakes, ElementsAre(Field(&Snake::id, "snake_id")));
  EXPECT_THAT(state.hazards, ElementsAreArray(expected_state.hazards));
}

TEST_F(ParseJsonTest, BoardStateWrongSnakesValueType) {
  auto json = nlohmann::json::parse(R"json(
        {
          "width": 5,
          "height": 15,
          "food": [],
          "snakes": {},
          "hazards": []
        }
    )json");

  EXPECT_THROW(ParseJsonBoard(json), ParseException);
}

TEST_F(ParseJsonTest, RulesetInfoSucceeds) {
  auto json = nlohmann::json::parse(
      R"json({"name": "standard", "version": "v1.2.3"})json");
  RulesetInfo expected_result{.name = "standard", .version = "v1.2.3"};

  RulesetInfo result = ParseJsonRulesetInfo(json);

  EXPECT_THAT(result.name, Eq(expected_result.name));
  EXPECT_THAT(result.version, Eq(expected_result.version));
}

TEST_F(ParseJsonTest, RulesetInfoWrongJsonType) {
  auto json = nlohmann::json::parse(
      R"json([{"name": "standard", "version": "v1.2.3"}])json");

  EXPECT_THROW(ParseJsonRulesetInfo(json), ParseException);
}

TEST_F(ParseJsonTest, GameInfoSucceeds) {
  auto json = nlohmann::json::parse(R"json({
      "id": "totally-unique-game-id",
      "ruleset": {
          "name": "standard",
          "version": "v1.2.3"
      },
      "timeout": 500
  })json");
  GameInfo expected_result{
      .id = "totally-unique-game-id",
      .ruleset{.name = "standard", .version = "v1.2.3"},
      .timeout = 500,
  };

  GameInfo result = ParseJsonGameInfo(json);

  EXPECT_THAT(result.id, Eq(expected_result.id));
  EXPECT_THAT(result.ruleset.name, Eq(expected_result.ruleset.name));
  EXPECT_THAT(result.ruleset.version, Eq(expected_result.ruleset.version));
  EXPECT_THAT(result.timeout, Eq(expected_result.timeout));
}

TEST_F(ParseJsonTest, GameInfoWrongJsonType) {
  auto json = nlohmann::json::parse(R"json([{
      "id": "totally-unique-game-id",
      "ruleset": {
          "name": "standard",
          "version": "v1.2.3"
      },
      "timeout": 500
  }])json");

  EXPECT_THROW(ParseJsonGameInfo(json), ParseException);
}

TEST_F(ParseJsonTest, GameInfoNoRuleset) {
  auto json = nlohmann::json::parse(R"json([{
      "id": "totally-unique-game-id",
      "timeout": 500
  }])json");

  EXPECT_THROW(ParseJsonGameInfo(json), ParseException);
}

TEST_F(ParseJsonTest, GameStateSucceeds) {
  auto json = nlohmann::json::parse(R"json({
        "game": {
            "id": "totally-unique-game-id",
            "ruleset": {
                "name": "standard",
                "version": "v1.2.3"
            },
            "timeout": 500
        },
        "turn": 987,
        "board": {
            "width": 5,
            "height": 15,
            "food": [],
            "snakes": [],
            "hazards": []
        },
        "you": {
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
  })json");
  GameState expected_result{
      .game{
          .id = "totally-unique-game-id",
          .ruleset{.name = "standard", .version = "v1.2.3"},
          .timeout = 500,
      },
      .turn = 987,
      .board{.width = 5, .height = 15},
      .you{
          .id = "snake_id",
          .body =
              {
                  Point{10, 1},
                  Point{10, 2},
                  Point{10, 3},
              },
          .health = 75,
          .name = "Test Caterpillar",
          .latency = "123",
          .shout = "Why are we shouting???",
          .squad = "The Suicide Squad",
      },
  };

  GameState result = ParseJsonGameState(json);

  // Check one subfield from each field.
  EXPECT_THAT(result.game.id, Eq(expected_result.game.id));
  EXPECT_THAT(result.turn, Eq(expected_result.turn));
  EXPECT_THAT(result.board.width, Eq(expected_result.board.width));
  EXPECT_THAT(result.you.id, Eq(expected_result.you.id));
}

TEST_F(ParseJsonTest, GameStateNoYou) {
  auto json = nlohmann::json::parse(R"json({
        "game": {
            "id": "totally-unique-game-id",
            "ruleset": {
                "name": "standard",
                "version": "v1.2.3"
            },
            "timeout": 500
        },
        "turn": 987,
        "board": {
            "width": 5,
            "height": 15,
            "food": [],
            "snakes": [],
            "hazards": []
        }
  })json");
  GameState expected_result{
      .game{
          .id = "totally-unique-game-id",
          .ruleset{.name = "standard", .version = "v1.2.3"},
          .timeout = 500,
      },
      .turn = 987,
      .board{.width = 5, .height = 15},
  };

  GameState result = ParseJsonGameState(json);

  // Check one subfield from each field.
  EXPECT_THAT(result.game.id, Eq(expected_result.game.id));
  EXPECT_THAT(result.turn, Eq(expected_result.turn));
  EXPECT_THAT(result.board.width, Eq(expected_result.board.width));
  EXPECT_THAT(result.you.id, Eq(expected_result.you.id));
}

TEST_F(ParseJsonTest, GameStateWrongJsonType) {
  auto json = nlohmann::json::parse(R"json([])json");

  EXPECT_THROW(ParseJsonGameState(json), ParseException);
}

TEST_F(ParseJsonTest, CustomizationSucceeds) {
  auto json = nlohmann::json::parse(R"json({
      "apiversion": "api_ver",
      "author": "a",
      "color": "#123456",
      "head": "h",
      "tail": "t",
      "version": "v"
    })json");

  Customization expected_result{
      .apiversion = "api_ver",
      .author = "a",
      .color = "#123456",
      .head = "h",
      .tail = "t",
      .version = "v",
  };

  Customization result = ParseJsonCustomization(json);

  EXPECT_THAT(result.apiversion, Eq(expected_result.apiversion));
  EXPECT_THAT(result.author, Eq(expected_result.author));
  EXPECT_THAT(result.color, Eq(expected_result.color));
  EXPECT_THAT(result.head, Eq(expected_result.head));
  EXPECT_THAT(result.tail, Eq(expected_result.tail));
  EXPECT_THAT(result.version, Eq(expected_result.version));
}

TEST_F(ParseJsonTest, CustomizationEmpty) {
  auto json = nlohmann::json::parse(R"json({})json");

  Customization expected_result{
      .apiversion = "",
      .author = "",
      .color = "",
      .head = "",
      .tail = "",
      .version = "",
  };

  Customization result = ParseJsonCustomization(json);

  EXPECT_THAT(result.apiversion, Eq(expected_result.apiversion));
  EXPECT_THAT(result.author, Eq(expected_result.author));
  EXPECT_THAT(result.color, Eq(expected_result.color));
  EXPECT_THAT(result.head, Eq(expected_result.head));
  EXPECT_THAT(result.tail, Eq(expected_result.tail));
  EXPECT_THAT(result.version, Eq(expected_result.version));
}

TEST_F(ParseJsonTest, CustomizationWrongJsonType) {
  auto json = nlohmann::json::parse(R"json([])json");

  EXPECT_THROW(ParseJsonCustomization(json), ParseException);
}

}  // namespace

}  // namespace json
}  // namespace battlesnake
