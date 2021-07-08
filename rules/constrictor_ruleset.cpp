#include "battlesnake/rules/constrictor_ruleset.h"

namespace battlesnake {
namespace rules {

BoardState ConstrictorRuleset::CreateInitialBoardState(
    int width, int height, std::vector<SnakeId> snake_ids) {
  BoardState next_state =
      StandardRuleset::CreateInitialBoardState(width, height, snake_ids);

  applyConstrictorRules(next_state);

  return next_state;
}

BoardState ConstrictorRuleset::CreateNextBoardState(
    const BoardState& prev_state,
    const std::unordered_map<SnakeId, Move>& moves, int turn) {
  BoardState next_state =
      StandardRuleset::CreateNextBoardState(prev_state, moves, turn);

  applyConstrictorRules(next_state);

  return next_state;
}

void ConstrictorRuleset::applyConstrictorRules(BoardState& state) const {
  state.food.clear();

  for (Snake& snake : state.snakes) {
    snake.health = snake_max_health_;

    if (snake.Length() < 2) {
      growSnake(snake);
      continue;
    }

    Point tail = snake.body[snake.body.size() - 1];
    Point sub_tail = snake.body[snake.body.size() - 2];
    if (tail != sub_tail) {
      growSnake(snake);
    }
  }
}

}  // namespace rules
}  // namespace battlesnake
