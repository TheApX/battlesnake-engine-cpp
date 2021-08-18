#pragma once

#include <trivial_loop_array.hpp>
#include <unordered_map>
#include <vector>

#include "battlesnake/rules/data_types.h"

namespace battlesnake {
namespace rules {

struct SnakeMove {
  SnakeId snake_id;
  Move move;
};

using SnakeMovesVector =
    ::theapx::trivial_loop_array<SnakeMove, kSnakesCountMax>;

class Ruleset {
 public:
  virtual ~Ruleset() = default;

  virtual BoardState CreateInitialBoardState(Coordinate width,
                                             Coordinate height,
                                             std::vector<SnakeId> snakeIDs) = 0;
  virtual void CreateNextBoardState(const BoardState& prev_state,
                                    const SnakeMovesVector& moves, int turn,
                                    BoardState& next_state) = 0;
  virtual bool IsGameOver(const BoardState& state) = 0;
};

}  // namespace rules
}  // namespace battlesnake
