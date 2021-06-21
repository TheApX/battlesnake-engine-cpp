#include <memory>
#include <nlohmann/json.hpp>

#include "battlesnake/rules/ruleset.h"

namespace battlesnake {
namespace json {

// Exception thrown on parsing errors.
class ParseException : public std::exception {
 public:
  explicit ParseException(const std::string& error = "Can't parse JSON")
      : error_(error) {}
  const char* what() const noexcept override { return error_.c_str(); }

 private:
  std::string error_;
};

// Create json for point.
nlohmann::json CreateJson(const battlesnake::engine::Point& point);
// Create json for non-eliminated snakes. Returns nullptr for eliminated snakes.
std::unique_ptr<nlohmann::json> MaybeCreateJson(
    const battlesnake::engine::Snake& snake);
// Create json for board.
nlohmann::json CreateJson(const battlesnake::engine::BoardState& state);

// Parses Point from json.
battlesnake::engine::Point ParseJsonPoint(const nlohmann::json& json);

// TODO:
// * ParseJsonSnake
// * ParseJsonBoard

}  // namespace json
}  // namespace battlesnake
