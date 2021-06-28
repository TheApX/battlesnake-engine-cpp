#pragma once

#include <exception>
#include <string>

namespace battlesnake {
namespace rules {

class RulesetException : public std::exception {
 public:
  explicit RulesetException(
      const std::string& error = "BattleSnake ruleset error")
      : error_(error) {}
  const char* what() const noexcept override { return error_.c_str(); }

 private:
  std::string error_;
};

class ErrorTooManySnakes : public RulesetException {
 public:
  ErrorTooManySnakes(size_t n)
      : RulesetException("Too many snakes for fixed start positions: " +
                         std::to_string(n)) {}
};

class ErrorNoRoomForSnake : public RulesetException {
 public:
  ErrorNoRoomForSnake() : RulesetException("Not enough space to place snake") {}
};

class ErrorNoRoomForFood : public RulesetException {
 public:
  ErrorNoRoomForFood() : RulesetException("Not enough space to place food") {}
};

class ErrorNoMoveFound : public RulesetException {
 public:
  ErrorNoMoveFound(const std::string& snake_id)
      : RulesetException("Move not provided for snake: '" + snake_id + "'") {}
};

class ErrorZeroLengthSnake : public RulesetException {
 public:
  ErrorZeroLengthSnake(const std::string& snake_id)
      : RulesetException("Snake is length zero: '" + snake_id + "'") {}
};

class ErrorInvalidEliminatedById : public RulesetException {
 public:
  ErrorInvalidEliminatedById(const std::string& snake_id,
                             const std::string& eliminated_by_id)
      : RulesetException("Invalid eliminated by id value '" + eliminated_by_id +
                         "' on snake '" + snake_id + "'") {}
};

}  // namespace rules
}  // namespace battlesnake
