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
  nlohmann::json json = CreateJson(Point{123, 21});
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({"x":123,"y":21})json")));
}

TEST_F(CreateJsonTest, EliminatedSnake) {
  EXPECT_THAT(MaybeCreateJson(Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::OutOfHealth}}),
              IsNull());
}

TEST_F(CreateJsonTest, NotEliminatedSnake) {
  StringPool pool;

  Snake snake{
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
      .hazard_info = {},
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
      .food = CreateBoardBits(
          {
              Point{0, 1},
              Point{4, 14},
          },
          5, 15),
      .snakes = {},
      .hazard_info = {},
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

TEST_F(CreateJsonTest, BoardStateHazardsLeft) {
  BoardState state{
      .width = 3,
      .height = 4,
      .food = {},
      .snakes = {},
      .hazard_info =
          {
              .depth_left = 1,
          },
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 3,
          "height": 4,
          "food": [],
          "snakes": [],
          "hazards": [
              {"x": 0, "y": 0},
              {"x": 0, "y": 1},
              {"x": 0, "y": 2},
              {"x": 0, "y": 3}
          ]
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateHazardsRight) {
  BoardState state{
      .width = 3,
      .height = 4,
      .food = {},
      .snakes = {},
      .hazard_info =
          {
              .depth_right = 2,
          },
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 3,
          "height": 4,
          "food": [],
          "snakes": [],
          "hazards": [
              {"x": 1, "y": 0},
              {"x": 2, "y": 0},
              {"x": 1, "y": 1},
              {"x": 2, "y": 1},
              {"x": 1, "y": 2},
              {"x": 2, "y": 2},
              {"x": 1, "y": 3},
              {"x": 2, "y": 3}
          ]
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateHazardsBottom) {
  BoardState state{
      .width = 3,
      .height = 4,
      .food = {},
      .snakes = {},
      .hazard_info =
          {
              .depth_bottom = 1,
          },
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 3,
          "height": 4,
          "food": [],
          "snakes": [],
          "hazards": [
              {"x": 0, "y": 0},
              {"x": 1, "y": 0},
              {"x": 2, "y": 0}
          ]
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateHazardsTop) {
  BoardState state{
      .width = 3,
      .height = 4,
      .food = {},
      .snakes = {},
      .hazard_info =
          {
              .depth_top = 2,
          },
  };

  auto expected_json = nlohmann::json::parse(R"json(
        {
          "width": 3,
          "height": 4,
          "food": [],
          "snakes": [],
          "hazards": [
              {"x": 0, "y": 2},
              {"x": 1, "y": 2},
              {"x": 2, "y": 2},
              {"x": 0, "y": 3},
              {"x": 1, "y": 3},
              {"x": 2, "y": 3}
          ]
        }
    )json");

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateHazardsGeneric) {
  BoardState state{
      .width = 5,
      .height = 6,
      .food = {},
      .snakes = {},
      .hazard_info =
          {
              .depth_left = 1,
              .depth_right = 2,
              .depth_top = 2,
              .depth_bottom = 1,
          },
  };

  auto expected_json = nlohmann::json::parse(R"json(
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

  EXPECT_THAT(CreateJson(state), expected_json);
}

TEST_F(CreateJsonTest, BoardStateSnakes) {
  StringPool pool;

  BoardState state{
      .width = 5,
      .height = 15,
      .food = {},
      .snakes = SnakesVector::Create({
          Snake{
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
      }),
      .hazard_info = {},
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
  StringPool pool;

  BoardState state{
      .width = 1,
      .height = 2,
      .food = {},
      .snakes = SnakesVector::Create({
          Snake{
              .id = pool.Add("snake_id"),
              .body = SnakeBody::Create({
                  Point{10, 1},
                  Point{10, 2},
                  Point{10, 3},
              }),
              .health = 75,
              .eliminated_cause =
                  EliminatedCause{
                      .cause = EliminatedCause::HeadToHeadCollision,
                  },
              .name = pool.Add("Test Caterpillar"),
              .latency = pool.Add("123"),
              .shout = pool.Add("Why are we shouting???"),
              .squad = pool.Add("The Suicide Squad"),
          },
      }),
      .hazard_info = {},
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

TEST_F(CreateJsonTest, RulesetSettings) {
  StringPool pool;

  nlohmann::json json = CreateJson(RulesetSettings{
      .food_spawn_chance = 15,
      .minimum_food = 1,
      .hazard_damage_per_turn = 30,
      .royale_shrink_every_n_turns = 123,
      .squad_allow_body_collisions = true,
      .squad_shared_elimination = false,
      .squad_shared_health = true,
      .squad_shared_length = false,
  });
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({
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
  })json")));
}

TEST_F(CreateJsonTest, RulesetInfo) {
  StringPool pool;

  nlohmann::json json = CreateJson(RulesetInfo{
      .name = pool.Add("standard"),
      .version = pool.Add("v1.2.3"),
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
  });
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({
    "name": "standard",
    "version": "v1.2.3",
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
  })json")));
}

TEST_F(CreateJsonTest, GameInfo) {
  StringPool pool;

  nlohmann::json json = CreateJson(GameInfo{
      .id = pool.Add("totally-unique-game-id"),
      .ruleset{
          .name = pool.Add("standard"),
          .version = pool.Add("v1.2.3"),
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
      },
      .timeout = 500,
  });
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({
      "id": "totally-unique-game-id",
      "ruleset": {
        "name": "standard",
        "version": "v1.2.3",
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
      },
      "timeout": 500
  })json")));
}

TEST_F(CreateJsonTest, GameState) {
  StringPool pool;

  nlohmann::json json = CreateJson(GameState{
      .game{
          .id = pool.Add("totally-unique-game-id"),
          .ruleset{
              .name = pool.Add("standard"),
              .version = pool.Add("v1.2.3"),
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
  });
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({
        "game": {
            "id": "totally-unique-game-id",
            "ruleset": {
                "name": "standard",
                "version": "v1.2.3",
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
  })json")));
}

TEST_F(CreateJsonTest, GameStateYouEliminated) {
  StringPool pool;

  nlohmann::json json = CreateJson(GameState{
      .game{
          .id = pool.Add("totally-unique-game-id"),
          .ruleset{
              .name = pool.Add("standard"),
              .version = pool.Add("v1.2.3"),
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
          .eliminated_cause{.cause = EliminatedCause::Collision},
          .name = pool.Add("Test Caterpillar"),
          .latency = pool.Add("123"),
          .shout = pool.Add("Why are we shouting???"),
          .squad = pool.Add("The Suicide Squad"),
      },
  });
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({
        "game": {
            "id": "totally-unique-game-id",
            "ruleset": {
                "name": "standard",
                "version": "v1.2.3",
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
  })json")));
}

TEST_F(CreateJsonTest, GameStateNoYou) {
  StringPool pool;

  nlohmann::json json = CreateJson(GameState{
      .game{
          .id = pool.Add("totally-unique-game-id"),
          .ruleset{
              .name = pool.Add("standard"),
              .version = pool.Add("v1.2.3"),
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
          },
          .timeout = 500,
      },
      .turn = 987,
      .board{.width = 5, .height = 15},
  });
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({
        "game": {
            "id": "totally-unique-game-id",
            "ruleset": {
                "name": "standard",
                "version": "v1.2.3",
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
  })json")));
}

TEST_F(CreateJsonTest, Customization) {
  Customization customization{
      .apiversion = "api_ver",
      .author = "a",
      .color = "#123456",
      .head = "h",
      .tail = "t",
      .version = "v",
  };
  nlohmann::json json = CreateJson(customization);
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({
      "apiversion": "api_ver",
      "author": "a",
      "color": "#123456",
      "head": "h",
      "tail": "t",
      "version": "v"
    })json")));
}

}  // namespace

}  // namespace json
}  // namespace battlesnake
