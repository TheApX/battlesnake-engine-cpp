#pragma once

#include <itlib/small_vector.hpp>
#include <unordered_map>
#include <vector>

#include "battlesnake/rules/data_types.h"

namespace battlesnake {
namespace rules {

using SnakeMovesVector =
    itlib::small_vector<std::pair<SnakeId, Move>, kOptimizeForMaxSnakesCount>;

class Ruleset {
 public:
  virtual ~Ruleset() = default;

  virtual BoardState CreateInitialBoardState(int width, int height,
                                             std::vector<SnakeId> snakeIDs) = 0;
  virtual void CreateNextBoardState(const BoardState& prev_state,
                                    const SnakeMovesVector& moves, int turn,
                                    BoardState& next_state) = 0;
  virtual bool IsGameOver(const BoardState& state) = 0;

  // Deprecated. Use the version that takes BoardState to save the result to.
  virtual BoardState CreateNextBoardState(
      const BoardState& prev_state,
      const std::unordered_map<SnakeId, Move>& moves, int turn) final {
    BoardState result;
    CreateNextBoardState(prev_state, moves, turn, result);
    return result;
  }

  // Deprecated. Use the version that takes moves in SnakeMovesVector.
  virtual void CreateNextBoardState(
      const BoardState& prev_state,
      const std::unordered_map<SnakeId, Move>& moves, int turn,
      BoardState& next_state) final {
    SnakeMovesVector moves_vector;
    moves_vector.reserve(moves.size());
    for (const auto& [snake_id, move] : moves) {
      moves_vector.push_back({snake_id, move});
    }
    return CreateNextBoardState(prev_state, moves_vector, turn, next_state);
  }
};

}  // namespace rules
}  // namespace battlesnake
