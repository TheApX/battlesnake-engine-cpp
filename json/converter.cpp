
#include "battlesnake/json/converter.h"

#include "battlesnake/rules/errors.h"

namespace battlesnake {
namespace json {
namespace {

using namespace ::battlesnake::rules;

template <class T>
nlohmann::json CreateContainerJson(const T& values) {
  nlohmann::json result = nlohmann::json::array();
  for (const auto& v : values) {
    result.push_back(CreateJson(v));
  }
  return result;
}

int GetInt(const nlohmann::json& json, const char* key) {
  auto v = json.find(key);
  if (v == json.end()) {
    throw ParseException();
  }
  if (!v->is_number_integer()) {
    throw ParseException();
  }
  return *v;
}

Coordinate GetCoordinate(const nlohmann::json& json, const char* key) {
  return static_cast<Coordinate>(GetInt(json, key));
}

StringWrapper GetString(const nlohmann::json& json, const char* key,
                        battlesnake::rules::StringPool& pool) {
  auto v = json.find(key);
  if (v == json.end()) {
    throw ParseException();
  }
  if (!v->is_string()) {
    throw ParseException();
  }
  return pool.Add(*v);
}

StringWrapper GetString(const nlohmann::json& json, const char* key,
                        const char* default_value,
                        battlesnake::rules::StringPool& pool) {
  auto v = json.find(key);
  if (v == json.end()) {
    return pool.Add(default_value);
  }
  if (!v->is_string()) {
    throw ParseException();
  }
  return pool.Add(*v);
}

std::string GetStringNoPool(const nlohmann::json& json, const char* key,
                            const char* default_value) {
  auto v = json.find(key);
  if (v == json.end()) {
    return default_value;
  }
  if (!v->is_string()) {
    throw ParseException();
  }
  return *v;
}

PointsVector GetPointArray(const nlohmann::json& json, const char* key) {
  auto v = json.find(key);
  if (v == json.end()) {
    throw ParseException();
  }
  if (!v->is_array()) {
    throw ParseException();
  }

  PointsVector result{};
  result.reserve(v->size());
  for (const nlohmann::json& p : *v) {
    result.push_back(ParseJsonPoint(p));
  }
  return result;
}

SnakesVector GetSnakeArray(const nlohmann::json& json, const char* key,
                           battlesnake::rules::StringPool& pool) {
  auto v = json.find(key);
  if (v == json.end()) {
    throw ParseException();
  }
  if (!v->is_array()) {
    throw ParseException();
  }

  SnakesVector result;
  result.reserve(v->size());
  for (const nlohmann::json& s : *v) {
    result.push_back(ParseJsonSnake(s, pool));
  }
  return result;
}

}  // namespace

nlohmann::json CreateJson(const Point& point) {
  return nlohmann::json{
      {"x", point.x},
      {"y", point.y},
  };
}

nlohmann::json CreateJson(const Snake& snake) {
  nlohmann::json result;
  result["id"] = snake.id.ToString();
  result["health"] = snake.health;
  result["head"] = CreateJson(snake.Head());
  result["body"] = CreateContainerJson(snake.body);
  result["length"] = snake.body.size();

  result["name"] = snake.name.ToString();
  result["latency"] = snake.latency.ToString();
  result["shout"] = snake.shout.ToString();
  result["squad"] = snake.squad.ToString();

  return result;
}

std::unique_ptr<nlohmann::json> MaybeCreateJson(const Snake& snake) {
  if (snake.IsEliminated()) {
    return nullptr;
  }

  return std::make_unique<nlohmann::json>(CreateJson(snake));
}

nlohmann::json CreateJson(const BoardState& state) {
  nlohmann::json result;

  result["width"] = state.width;
  result["height"] = state.height;
  result["food"] = CreateContainerJson(state.food);
  result["hazards"] = CreateContainerJson(state.hazards);

  auto snakes = nlohmann::json::array();
  for (const Snake& snake : state.snakes) {
    auto snake_json = MaybeCreateJson(snake);
    if (snake_json == nullptr) {
      continue;
    }
    snakes.push_back(*snake_json);
  }
  result["snakes"] = std::move(snakes);

  return result;
}

nlohmann::json CreateJson(const RulesetInfo& ruleset_info) {
  return nlohmann::json{
      {"name", ruleset_info.name.ToString()},
      {"version", ruleset_info.version.ToString()},
  };
}

nlohmann::json CreateJson(const GameInfo& game_info) {
  return nlohmann::json{
      {"id", game_info.id.ToString()},
      {"ruleset", CreateJson(game_info.ruleset)},
      {"timeout", game_info.timeout},
  };
}

nlohmann::json CreateJson(const GameState& game_state) {
  auto result = nlohmann::json{
      {"game", CreateJson(game_state.game)},
      {"turn", game_state.turn},
      {"board", CreateJson(game_state.board)},
  };
  if (game_state.you.Length() > 0 && !game_state.you.IsEliminated()) {
    result["you"] = CreateJson(game_state.you);
  }
  return result;
}

nlohmann::json CreateJson(const Customization& customization) {
  return nlohmann::json{
      {"apiversion", customization.apiversion},
      {"author", customization.author},
      {"color", customization.color},
      {"head", customization.head},
      {"tail", customization.tail},
      {"version", customization.version},
  };
}

Point ParseJsonPoint(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }
  return Point{GetCoordinate(json, "x"), GetCoordinate(json, "y")};
}

Snake ParseJsonSnake(const nlohmann::json& json,
                     battlesnake::rules::StringPool& pool) {
  if (!json.is_object()) {
    throw ParseException();
  }

  Snake snake{
      .id = GetString(json, "id", pool),
      .body = SnakeBody::Create(GetPointArray(json, "body")),
      .health = GetInt(json, "health"),
      .name = GetString(json, "name", "", pool),
      .latency = GetString(json, "latency", "0", pool),
      .shout = GetString(json, "shout", "", pool),
      .squad = GetString(json, "squad", "", pool),
  };

  Point head = ParseJsonPoint(json["head"]);
  if (snake.body.empty()) {
    throw ErrorZeroLengthSnake(snake.id.ToString());
  }

  if (head != snake.Head()) {
    throw ParseException("Different head values");
  }

  return snake;
}

BoardState ParseJsonBoard(const nlohmann::json& json,
                          battlesnake::rules::StringPool& pool) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return BoardState{
      .width = GetCoordinate(json, "width"),
      .height = GetCoordinate(json, "height"),
      .food = GetPointArray(json, "food"),
      .snakes = GetSnakeArray(json, "snakes", pool),
      .hazards = GetPointArray(json, "hazards"),
  };
}

RulesetInfo ParseJsonRulesetInfo(const nlohmann::json& json,
                                 battlesnake::rules::StringPool& pool) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return RulesetInfo{
      .name = GetString(json, "name", pool),
      .version = GetString(json, "version", pool),
  };
}

GameInfo ParseJsonGameInfo(const nlohmann::json& json,
                           battlesnake::rules::StringPool& pool) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return GameInfo{
      .id = GetString(json, "id", pool),
      .ruleset = ParseJsonRulesetInfo(json["ruleset"], pool),
      .timeout = GetInt(json, "timeout"),
  };
}

battlesnake::rules::GameState ParseJsonGameState(
    const nlohmann::json& json, battlesnake::rules::StringPool& pool) {
  if (!json.is_object()) {
    throw ParseException();
  }

  GameState game_state{
      .game = ParseJsonGameInfo(json["game"], pool),
      .turn = GetInt(json, "turn"),
      .board = ParseJsonBoard(json["board"], pool),
  };

  auto you_it = json.find("you");
  if (you_it != json.end()) {
    game_state.you = ParseJsonSnake(*you_it, pool);
  }

  return game_state;
}

Customization ParseJsonCustomization(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return Customization{
      .apiversion = GetStringNoPool(json, "apiversion", ""),
      .author = GetStringNoPool(json, "author", ""),
      .color = GetStringNoPool(json, "color", ""),
      .head = GetStringNoPool(json, "head", ""),
      .tail = GetStringNoPool(json, "tail", ""),
      .version = GetStringNoPool(json, "version", ""),
  };
}

}  // namespace json
}  // namespace battlesnake
