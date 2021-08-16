#pragma once

#include <unordered_map>
#include <vector>

#include "battlesnake/rules/data_types.h"

namespace battlesnake {
namespace rules {

class Ruleset {
 public:
  virtual ~Ruleset() = default;

  virtual BoardState CreateInitialBoardState(int width, int height,
                                             std::vector<SnakeId> snakeIDs) = 0;
  virtual void CreateNextBoardState(
      const BoardState& prev_state,
      const std::unordered_map<SnakeId, Move>& moves, int turn,
      BoardState& next_state) = 0;
  virtual bool IsGameOver(const BoardState& state) = 0;

  // Deprecated. Use the version that takes BoardState to save the result to.
  virtual BoardState CreateNextBoardState(
      const BoardState& prev_state,
      const std::unordered_map<SnakeId, Move>& moves, int turn) {
    BoardState result;
    CreateNextBoardState(prev_state, moves, turn, result);
    return result;
  }
};

}  // namespace rules
}  // namespace battlesnake
