#include <memory>
#include <nlohmann/json.hpp>

#include "battlesnake/rules/data_types.h"

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
nlohmann::json CreateJson(const battlesnake::rules::Point& point);
// Create json for non-eliminated snakes. Returns nullptr for eliminated snakes.
std::unique_ptr<nlohmann::json> MaybeCreateJson(
    const battlesnake::rules::Snake& snake);
// Create json for board.
nlohmann::json CreateJson(const battlesnake::rules::BoardState& state);

// Parses Point from json.
battlesnake::rules::Point ParseJsonPoint(const nlohmann::json& json);
// Parses Snake from json.
battlesnake::rules::Snake ParseJsonSnake(const nlohmann::json& json);
// Parses BoardState from json.
battlesnake::rules::BoardState ParseJsonBoard(const nlohmann::json& json);

}  // namespace json
}  // namespace battlesnake
