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

}  // namespace json
}  // namespace battlesnake
