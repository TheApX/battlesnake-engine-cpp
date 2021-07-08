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
  virtual BoardState CreateNextBoardState(
      const BoardState& prev_state,
      const std::unordered_map<SnakeId, Move>& moves, int turn) = 0;
  virtual bool IsGameOver(const BoardState& state) = 0;
};

}  // namespace rules
}  // namespace battlesnake
