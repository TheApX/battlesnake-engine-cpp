
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

std::string GetString(const nlohmann::json& json, const char* key) {
  auto v = json.find(key);
  if (v == json.end()) {
    throw ParseException();
  }
  if (!v->is_string()) {
    throw ParseException();
  }
  return *v;
}

std::string GetString(const nlohmann::json& json, const char* key,
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

  PointsVector result;
  result.reserve(v->size());
  for (const nlohmann::json& p : *v) {
    result.push_back(ParseJsonPoint(p));
  }
  return result;
}

SnakesVector GetSnakeArray(const nlohmann::json& json, const char* key) {
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
    result.push_back(ParseJsonSnake(s));
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
  result["id"] = snake.id;
  result["health"] = snake.health;
  result["head"] = CreateJson(snake.Head());
  result["body"] = CreateContainerJson(snake.body);
  result["length"] = snake.body.size();

  result["name"] = snake.name;
  result["latency"] = snake.latency;
  result["shout"] = snake.shout;
  result["squad"] = snake.squad;

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
      {"name", ruleset_info.name},
      {"version", ruleset_info.version},
  };
}

nlohmann::json CreateJson(const GameInfo& game_info) {
  return nlohmann::json{
      {"id", game_info.id},
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
  return Point{GetInt(json, "x"), GetInt(json, "y")};
}

Snake ParseJsonSnake(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }

  Snake snake{
      .id = GetString(json, "id"),
      .body = GetPointArray(json, "body"),
      .health = GetInt(json, "health"),
      .name = GetString(json, "name", ""),
      .latency = GetString(json, "latency", "0"),
      .shout = GetString(json, "shout", ""),
      .squad = GetString(json, "squad", ""),
  };

  Point head = ParseJsonPoint(json["head"]);
  if (snake.body.empty()) {
    throw ErrorZeroLengthSnake(snake.id);
  }

  if (head != snake.Head()) {
    throw ParseException("Different head values");
  }

  return snake;
}

BoardState ParseJsonBoard(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return BoardState{
      .width = GetInt(json, "width"),
      .height = GetInt(json, "height"),
      .food = GetPointArray(json, "food"),
      .snakes = GetSnakeArray(json, "snakes"),
      .hazards = GetPointArray(json, "hazards"),
  };
}

RulesetInfo ParseJsonRulesetInfo(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return RulesetInfo{
      .name = GetString(json, "name"),
      .version = GetString(json, "version"),
  };
}

GameInfo ParseJsonGameInfo(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return GameInfo{
      .id = GetString(json, "id"),
      .ruleset = ParseJsonRulesetInfo(json["ruleset"]),
      .timeout = GetInt(json, "timeout"),
  };
}

battlesnake::rules::GameState ParseJsonGameState(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }

  GameState game_state{
      .game = ParseJsonGameInfo(json["game"]),
      .turn = GetInt(json, "turn"),
      .board = ParseJsonBoard(json["board"]),
  };

  auto you_it = json.find("you");
  if (you_it != json.end()) {
    game_state.you = ParseJsonSnake(*you_it);
  }

  return game_state;
}

Customization ParseJsonCustomization(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }

  return Customization{
      .apiversion = GetString(json, "apiversion", ""),
      .author = GetString(json, "author", ""),
      .color = GetString(json, "color", ""),
      .head = GetString(json, "head", ""),
      .tail = GetString(json, "tail", ""),
      .version = GetString(json, "version", ""),
  };
}

}  // namespace json
}  // namespace battlesnake
