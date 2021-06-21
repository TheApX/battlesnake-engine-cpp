#include "battlesnake/json/converter.h"

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
  return Point(GetInt(json, "x"), GetInt(json, "y"));
}

}  // namespace json
}  // namespace battlesnake
