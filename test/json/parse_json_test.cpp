#include <vector>

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

std::vector<Point> BoardBitsVector(const BoardBitsView& food) {
  std::vector<Point> result;
  for (const Point& p : food) {
    result.push_back(p);
  }
  return result;
}

class ParseJsonTest : public testing::Test {};

TEST_F(ParseJsonTest, PointSucceeds) {
  EXPECT_THAT(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"y":21})json")),
      Eq(Point{123, 21}));
}

TEST_F(ParseJsonTest, PointNoValue) {
  EXPECT_THROW(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"z":21})json")),
      ParseException);
}

TEST_F(ParseJsonTest, PointInvalidJsonType) {
  EXPECT_THROW(
      ParseJsonPoint(nlohmann::json::parse(R"json([{"x":123,"y":21}])json")),
      ParseException);
}

TEST_F(ParseJsonTest, PointInvalidValueType) {
  EXPECT_THROW(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"y":"21"})json")),
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

  StringPool pool;
  Snake expected_snake{
      .id = pool.Add("snake_id"),
      .body = SnakeBody::Create({
          Point{10, 1},
          Point{10, 2},
          Point{10, 3},
      }),
      .health = 75,
      .name = pool.Add("Test Caterpillar"),
      .latency = pool.Add("123"),
      .shout = pool.Add("Why are we shouting???"),
      .squad = pool.Add("The Suicide Squad"),
  };

  Snake snake = ParseJsonSnake(json, pool);

  EXPECT_THAT(snake.id, Eq(expected_snake.id));
  EXPECT_THAT(snake.body, Eq(expected_snake.body));
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

  StringPool pool;
  Snake expected_snake{
      .id = pool.Add("snake_id"),
      .body = SnakeBody::Create({
          Point{10, 1},
          Point{10, 2},
          Point{10, 3},
      }),
      .health = 75,
      .latency = pool.Add("0"),
  };

  Snake snake = ParseJsonSnake(json, pool);

  EXPECT_THAT(snake.id, Eq(expected_snake.id));
  EXPECT_THAT(snake.body, Eq(expected_snake.body));
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

  StringPool pool;
  EXPECT_THROW(ParseJsonSnake(json, pool), ParseException);
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

  StringPool pool;
  EXPECT_THROW(ParseJsonSnake(json, pool), ParseException);
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

  StringPool pool;
  EXPECT_THROW(ParseJsonSnake(json, pool), ParseException);
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
      .hazard = {},
  };

  StringPool pool;
  BoardState state = ParseJsonBoard(json, pool);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.Food(),
              ElementsAreArray(BoardBitsVector(expected_state.Food())));
  EXPECT_THAT(state.snakes, ElementsAre());
  EXPECT_THAT(state.Hazard(),
              ElementsAreArray(BoardBitsVector(expected_state.Hazard())));
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
      .food = CreateBoardBits(
          {
              Point{0, 1},
              Point{4, 14},
          },
          5, 15),
      .hazard = {},
  };

  StringPool pool;
  BoardState state = ParseJsonBoard(json, pool);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.Food(),
              ElementsAreArray(BoardBitsVector(expected_state.Food())));
  EXPECT_THAT(state.snakes, ElementsAre());
  EXPECT_THAT(state.Hazard(),
              ElementsAreArray(BoardBitsVector(expected_state.Hazard())));
}

TEST_F(ParseJsonTest, BoardStateHazards) {
  auto json = nlohmann::json::parse(R"json(
        {
          "width": 5,
          "height": 6,
          "food": [],
          "snakes": [],
          "hazards": [
              {"x": 0, "y": 0},
              {"x": 1, "y": 0},
              {"x": 2, "y": 0},
              {"x": 3, "y": 0},
              {"x": 4, "y": 0},

              {"x": 0, "y": 1},
              {"x": 3, "y": 1},
              {"x": 4, "y": 1},

              {"x": 0, "y": 2},
              {"x": 3, "y": 2},
              {"x": 4, "y": 2},

              {"x": 0, "y": 3},
              {"x": 3, "y": 3},
              {"x": 4, "y": 3},

              {"x": 0, "y": 4},
              {"x": 1, "y": 4},
              {"x": 2, "y": 4},
              {"x": 3, "y": 4},
              {"x": 4, "y": 4},

              {"x": 0, "y": 5},
              {"x": 1, "y": 5},
              {"x": 2, "y": 5},
              {"x": 3, "y": 5},
              {"x": 4, "y": 5}
          ]
        }
    )json");

  BoardState expected_state{
      .width = 5,
      .height = 6,
      .food = {},
      .snakes = {},
      .hazard = CreateBoardBits(
          {
              Point{0, 0},  //
              Point{1, 0},  //
              Point{2, 0},  //
              Point{3, 0},  //
              Point{4, 0},  //

              Point{0, 1},  //
              Point{3, 1},  //
              Point{4, 1},  //

              Point{0, 2},  //
              Point{3, 2},  //
              Point{4, 2},  //

              Point{0, 3},  //
              Point{3, 3},  //
              Point{4, 3},  //

              Point{0, 4},  //
              Point{1, 4},  //
              Point{2, 4},  //
              Point{3, 4},  //
              Point{4, 4},  //

              Point{0, 5},  //
              Point{1, 5},  //
              Point{2, 5},  //
              Point{3, 5},  //
              Point{4, 5},  //
          },
          5, 6),
  };

  StringPool pool;
  BoardState state = ParseJsonBoard(json, pool);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.Food(),
              ElementsAreArray(BoardBitsVector(expected_state.Food())));
  EXPECT_THAT(state.snakes, ElementsAre());
  EXPECT_THAT(state.Hazard(),
              ElementsAreArray(BoardBitsVector(expected_state.Hazard())));
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
      .hazard = {},
  };

  StringPool pool;
  BoardState state = ParseJsonBoard(json, pool);

  EXPECT_THAT(state.width, Eq(expected_state.width));
  EXPECT_THAT(state.height, Eq(expected_state.height));
  EXPECT_THAT(state.Food(),
              ElementsAreArray(BoardBitsVector(expected_state.Food())));
  EXPECT_THAT(state.snakes, ElementsAre(Field(&Snake::id, "snake_id")));
  EXPECT_THAT(state.Hazard(),
              ElementsAreArray(BoardBitsVector(expected_state.Hazard())));
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

  StringPool pool;
  EXPECT_THROW(ParseJsonBoard(json, pool), ParseException);
}

TEST_F(ParseJsonTest, RulesetSettingsSucceeds) {
  auto json = nlohmann::json::parse(R"json({
      "foodSpawnChance": 15,
      "minimumFood": 1,
      "hazardDamagePerTurn": 30,
      "royale": {
        "shrinkEveryNTurns": 123
      },
      "squad": {
        "allowBodyCollisions": true,
        "sharedElimination": false,
        "sharedHealth": true,
        "sharedLength": false
      }
    })json");

  RulesetSettings expected_result{
      .food_spawn_chance = 15,
      .minimum_food = 1,
      .hazard_damage_per_turn = 30,
      .royale_shrink_every_n_turns = 123,
      .squad_allow_body_collisions = true,
      .squad_shared_elimination = false,
      .squad_shared_health = true,
      .squad_shared_length = false,
  };

  RulesetSettings result = ParseJsonRulesetSettings(json);

  EXPECT_THAT(result.food_spawn_chance, Eq(expected_result.food_spawn_chance));
  EXPECT_THAT(result.minimum_food, Eq(expected_result.minimum_food));
  EXPECT_THAT(result.hazard_damage_per_turn,
              Eq(expected_result.hazard_damage_per_turn));
  EXPECT_THAT(result.royale_shrink_every_n_turns,
              Eq(expected_result.royale_shrink_every_n_turns));
  EXPECT_THAT(result.squad_allow_body_collisions,
              Eq(expected_result.squad_allow_body_collisions));
  EXPECT_THAT(result.squad_shared_elimination,
              Eq(expected_result.squad_shared_elimination));
  EXPECT_THAT(result.squad_shared_health,
              Eq(expected_result.squad_shared_health));
  EXPECT_THAT(result.squad_shared_length,
              Eq(expected_result.squad_shared_length));
}

TEST_F(ParseJsonTest, RulesetSettingsWrongJsonType) {
  auto json = nlohmann::json::parse(R"json([])json");

  EXPECT_THROW(ParseJsonRulesetSettings(json), ParseException);
}

TEST_F(ParseJsonTest, RulesetInfoSucceeds) {
  auto json = nlohmann::json::parse(R"json({
      "name": "standard",
      "version": "v1.0.21",
      "settings": {
        "foodSpawnChance": 15,
        "minimumFood": 1,
        "hazardDamagePerTurn": 30,
        "royale": {
          "shrinkEveryNTurns": 123
        },
        "squad": {
          "allowBodyCollisions": true,
          "sharedElimination": false,
          "sharedHealth": true,
          "sharedLength": false
        }
      }
    })json");

  StringPool pool;
  RulesetInfo expected_result{
      .name = pool.Add("standard"),
      .version = pool.Add("v1.0.21"),
      .settings =
          {
              .food_spawn_chance = 15,
              .minimum_food = 1,
              .hazard_damage_per_turn = 30,
              .royale_shrink_every_n_turns = 123,
              .squad_allow_body_collisions = true,
              .squad_shared_elimination = false,
              .squad_shared_health = true,
              .squad_shared_length = false,
          },
  };

  RulesetInfo result = ParseJsonRulesetInfo(json, pool);

  EXPECT_THAT(result.name, Eq(expected_result.name));
  EXPECT_THAT(result.version, Eq(expected_result.version));
  EXPECT_THAT(result.settings.food_spawn_chance,
              Eq(expected_result.settings.food_spawn_chance));
}

TEST_F(ParseJsonTest, RulesetInfoWrongJsonType) {
  auto json = nlohmann::json::parse(
      R"json([{"name": "standard", "version": "v1.2.3"}])json");

  StringPool pool;
  EXPECT_THROW(ParseJsonRulesetInfo(json, pool), ParseException);
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

  StringPool pool;
  GameInfo expected_result{
      .id = pool.Add("totally-unique-game-id"),
      .ruleset{
          .name = pool.Add("standard"),
          .version = pool.Add("v1.2.3"),
      },
      .timeout = 500,
  };

  GameInfo result = ParseJsonGameInfo(json, pool);

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

  StringPool pool;
  EXPECT_THROW(ParseJsonGameInfo(json, pool), ParseException);
}

TEST_F(ParseJsonTest, GameInfoNoRuleset) {
  auto json = nlohmann::json::parse(R"json([{
      "id": "totally-unique-game-id",
      "timeout": 500
  }])json");

  StringPool pool;
  EXPECT_THROW(ParseJsonGameInfo(json, pool), ParseException);
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

  StringPool pool;
  GameState expected_result{
      .game{
          .id = pool.Add("totally-unique-game-id"),
          .ruleset{
              .name = pool.Add("standard"),
              .version = pool.Add("v1.2.3"),
          },
          .timeout = 500,
      },
      .turn = 987,
      .board{.width = 5, .height = 15},
      .you{
          .id = pool.Add("snake_id"),
          .body = SnakeBody::Create({
              Point{10, 1},
              Point{10, 2},
              Point{10, 3},
          }),
          .health = 75,
          .name = pool.Add("Test Caterpillar"),
          .latency = pool.Add("123"),
          .shout = pool.Add("Why are we shouting???"),
          .squad = pool.Add("The Suicide Squad"),
      },
  };

  GameState result = ParseJsonGameState(json, pool);

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

  StringPool pool;
  GameState expected_result{
      .game{
          .id = pool.Add("totally-unique-game-id"),
          .ruleset{
              .name = pool.Add("standard"),
              .version = pool.Add("v1.2.3"),
          },
          .timeout = 500,
      },
      .turn = 987,
      .board{.width = 5, .height = 15},
  };

  GameState result = ParseJsonGameState(json, pool);

  // Check one subfield from each field.
  EXPECT_THAT(result.game.id, Eq(expected_result.game.id));
  EXPECT_THAT(result.turn, Eq(expected_result.turn));
  EXPECT_THAT(result.board.width, Eq(expected_result.board.width));
  EXPECT_THAT(result.you.id, Eq(expected_result.you.id));
}

TEST_F(ParseJsonTest, GameStateWrongJsonType) {
  auto json = nlohmann::json::parse(R"json([])json");

  StringPool pool;
  EXPECT_THROW(ParseJsonGameState(json, pool), ParseException);
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
