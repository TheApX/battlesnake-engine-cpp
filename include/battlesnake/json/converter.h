#include <memory>
#include <nlohmann/json.hpp>

#include "battlesnake/rules/ruleset.h"

namespace battlesnake {
namespace json {

// Create json for point.
nlohmann::json CreateJson(const battlesnake::engine::Point& point);
// Create json for non-eliminated snakes. Returns nullptr for eliminated snakes.
std::unique_ptr<nlohmann::json> MayCreateJson(
    const battlesnake::engine::Snake& snake);

}  // namespace json
}  // namespace battlesnake
