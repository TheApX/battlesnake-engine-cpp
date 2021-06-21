#include <nlohmann/json.hpp>

#include "battlesnake/rules/ruleset.h"

namespace battlesnake {
namespace json {

nlohmann::json CreateJson(const battlesnake::engine::Point& point);

}  // namespace json
}  // namespace battlesnake
