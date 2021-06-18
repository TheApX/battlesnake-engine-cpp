#include "standard_ruleset.h"

namespace battlesnake {
namespace engine {

BoardState StandardRuleset::CreateInitialBoardState(
    int width, int height, std::vector<SnakeId> snakeIDs) {
  BoardState initial_board_state;

  return initial_board_state;
}

BoardState StandardRuleset::CreateNextBoardState(
    const BoardState& prev_state, std::map<SnakeId, Move> moves) {
  BoardState next_state;

  return next_state;
}

bool StandardRuleset::IsGameOver(const BoardState& state) { return true; }

}  // namespace engine
}  // namespace battlesnake
