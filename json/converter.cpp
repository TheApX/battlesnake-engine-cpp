
#include "battlesnake/json/converter.h"

#include "battlesnake/rules/ruleset_errors.h"

namespace battlesnake {
namespace json {
namespace {

using namespace ::battlesnake::engine;

template <class T>
nlohmann::json CreateVectorJson(const std::vector<T>& values) {
  nlohmann::json result = nlohmann::json::array();
  for (const T& v : values) {
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

std::vector<Point> GetPointArray(const nlohmann::json& json, const char* key) {
  auto v = json.find(key);
  if (v == json.end()) {
    throw ParseException();
  }
  if (!v->is_array()) {
    throw ParseException();
  }

  std::vector<Point> result;
  result.reserve(v->size());
  for (const nlohmann::json& p : *v) {
    result.push_back(ParseJsonPoint(p));
  }
  return result;
}

std::vector<Snake> GetSnakeArray(const nlohmann::json& json, const char* key) {
  auto v = json.find(key);
  if (v == json.end()) {
    throw ParseException();
  }
  if (!v->is_array()) {
    throw ParseException();
  }

  std::vector<Snake> result;
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

std::unique_ptr<nlohmann::json> MaybeCreateJson(const Snake& snake) {
  if (snake.IsEliminated()) {
    return nullptr;
  }

  auto result_ptr = std::make_unique<nlohmann::json>();

  nlohmann::json& result = *result_ptr;
  result["id"] = snake.id;
  result["health"] = snake.health;
  result["head"] = CreateJson(snake.Head());
  result["body"] = CreateVectorJson(snake.body);
  result["length"] = snake.body.size();

  result["name"] = snake.name;
  result["latency"] = snake.latency;
  result["shout"] = snake.shout;
  result["squad"] = snake.squad;

  return result_ptr;
}

nlohmann::json CreateJson(const BoardState& state) {
  nlohmann::json result;

  result["width"] = state.width;
  result["height"] = state.height;
  result["food"] = CreateVectorJson(state.food);
  result["hazards"] = CreateVectorJson(state.hazards);

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

Point ParseJsonPoint(const nlohmann::json& json) {
  if (!json.is_object()) {
    throw ParseException();
  }
  return Point(GetInt(json, "x"), GetInt(json, "y"));
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

}  // namespace json
}  // namespace battlesnake
