#include "battlesnake/rules/constrictor_ruleset.h"

namespace battlesnake {
namespace rules {

BoardState ConstrictorRuleset::CreateInitialBoardState(
    Coordinate width, Coordinate height, std::vector<SnakeId> snake_ids) {
  BoardState next_state =
      StandardRuleset::CreateInitialBoardState(width, height, snake_ids);

  applyConstrictorRules(next_state);

  return next_state;
}

void ConstrictorRuleset::CreateNextBoardState(const BoardState& prev_state,
                                              const SnakeMovesVector& moves,
                                              int turn,
                                              BoardState& next_state) {
  StandardRuleset::CreateNextBoardState(prev_state, moves, turn, next_state);

  applyConstrictorRules(next_state);
}

void ConstrictorRuleset::applyConstrictorRules(BoardState& state) const {
  state.food.clear();

  for (Snake& snake : state.snakes) {
    snake.health = snake_max_health_;

    if (snake.Length() < 2) {
      growSnake(snake);
      continue;
    }

    if (snake.body.moves_length == snake.body.total_length - 1) {
      growSnake(snake);
    }
  }
}

}  // namespace rules
}  // namespace battlesnake
