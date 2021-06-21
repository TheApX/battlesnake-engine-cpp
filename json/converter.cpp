#include "battlesnake/json/converter.h"

namespace battlesnake {
namespace json {
namespace {

using namespace ::battlesnake::engine;
using json = nlohmann::json;

}  // namespace

nlohmann::json CreateJson(const battlesnake::engine::Point& point) {
  return json{
      {"x", point.x},
      {"y", point.y},
  };
}

std::unique_ptr<nlohmann::json> MayCreateJson(
    const battlesnake::engine::Snake& snake) {
  if (snake.IsEliminated()) {
    return nullptr;
  }

  auto result_ptr = std::make_unique<nlohmann::json>();

  nlohmann::json& result = *result_ptr;
  result["id"] = snake.id;
  result["health"] = snake.health;
  result["head"] = CreateJson(snake.Head());

  json body = json::array();
  for (const Point& p : snake.body) {
    body.push_back(CreateJson(p));
  }
  result["body"] = std::move(body);
  result["length"] = snake.body.size();

  result["name"] = snake.name;
  result["latency"] = snake.latency;
  result["shout"] = snake.shout;
  result["squad"] = snake.squad;

  return result_ptr;
}

}  // namespace json
}  // namespace battlesnake
