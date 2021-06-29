#pragma once

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
// Create json for snake. Ignores elimination cause.
nlohmann::json CreateJson(const battlesnake::rules::Snake& snake);
// Create json for non-eliminated snakes. Returns nullptr for eliminated snakes.
std::unique_ptr<nlohmann::json> MaybeCreateJson(
    const battlesnake::rules::Snake& snake);
// Create json for board.
nlohmann::json CreateJson(const battlesnake::rules::BoardState& state);
// Create json for RulesetInfo.
nlohmann::json CreateJson(const battlesnake::rules::RulesetInfo& ruleset_info);
// Create json for GameInfo.
nlohmann::json CreateJson(const battlesnake::rules::GameInfo& game_info);
// Create json for GameState.
nlohmann::json CreateJson(const battlesnake::rules::GameState& game_state);
// Create json for Customization.
nlohmann::json CreateJson(
    const battlesnake::rules::Customization& customization);

// Parses Point from json.
battlesnake::rules::Point ParseJsonPoint(const nlohmann::json& json);
// Parses Snake from json.
battlesnake::rules::Snake ParseJsonSnake(const nlohmann::json& json);
// Parses BoardState from json.
battlesnake::rules::BoardState ParseJsonBoard(const nlohmann::json& json);
// Parses RulesetInfo from json.
battlesnake::rules::RulesetInfo ParseJsonRulesetInfo(
    const nlohmann::json& json);
// Parses GameInfo from json.
battlesnake::rules::GameInfo ParseJsonGameInfo(const nlohmann::json& json);
// Parses GameState from json.
battlesnake::rules::GameState ParseJsonGameState(const nlohmann::json& json);
// Parses Customization from json.
battlesnake::rules::Customization ParseJsonCustomization(
    const nlohmann::json& json);

}  // namespace json
}  // namespace battlesnake
