#include "battlesnake/rules/royale_ruleset.h"

#include <algorithm>

#include "battlesnake/rules/errors.h"

namespace battlesnake {
namespace rules {

void RoyaleRuleset::CreateNextBoardState(const BoardState& prev_state,
                                         const SnakeMovesVector& moves,
                                         int turn, BoardState& next_state) {
  StandardRuleset::CreateNextBoardState(prev_state, moves, turn, next_state);

  Bounds bounds = findBounds(next_state);
  damageOutOfBounds(bounds, next_state);
  if (maybeShrinkBounds(turn, bounds)) {
    fillHazards(bounds, next_state);
  }
}

RoyaleRuleset::Bounds RoyaleRuleset::findBounds(const BoardState& state) const {
  return Bounds{
      .min_x = state.hazard_info.depth_left,
      .max_x = state.width - state.hazard_info.depth_right - 1,
      .min_y = state.hazard_info.depth_bottom,
      .max_y = state.height - state.hazard_info.depth_top - 1,
  };
}

void RoyaleRuleset::damageOutOfBounds(const Bounds& bounds,
                                      BoardState& state) const {
  for (Snake& snake : state.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }
    if (snake.Length() == 0) {
      continue;
    }

    Point head = snake.Head();

    bool in_bounds = true                         //
                     && (head.x >= bounds.min_x)  //
                     && (head.x <= bounds.max_x)  //
                     && (head.y >= bounds.min_y)  //
                     && (head.y <= bounds.max_y);
    if (in_bounds) {
      continue;
    }

    if (snake.health != config_.snake_max_health) {
      // Decrease health only if no food was eaten. Detect it by health being at
      // max level.
      snake.health -= royale_config_.extra_damage_per_turn;
    }
    if (snake.IsOutOfHealth()) {
      snake.health = 0;
      snake.eliminated_cause.cause = EliminatedCause::OutOfHealth;
    }
  }
}

bool RoyaleRuleset::maybeShrinkBounds(int turn,
                                      RoyaleRuleset::Bounds& bounds) const {
  if (turn % royale_config_.shrink_every_n_turns != 0) {
    return false;
  }
  if (bounds.max_x < bounds.min_x) {
    return false;
  }
  if (bounds.max_y < bounds.min_y) {
    return false;
  }

  int r = getRandomNumber(4);
  switch (r) {
    case 0:
      // Shrink on the left.
      bounds.min_x++;
      break;

    case 1:
      // Shrink on the right.
      bounds.max_x--;
      break;

    case 2:
      // Shrink on the bottom.
      bounds.min_y++;
      break;

    case 3:
      // Shrink on the top.
      bounds.max_y--;
      break;

    default:
      return false;
  }

  return true;
}

void RoyaleRuleset::fillHazards(const Bounds& bounds, BoardState& state) const {
  state.hazard_info.depth_left = bounds.min_x;
  state.hazard_info.depth_right = state.width - bounds.max_x - 1;
  state.hazard_info.depth_bottom = bounds.min_y;
  state.hazard_info.depth_top = state.height - bounds.max_y - 1;
}

}  // namespace rules
}  // namespace battlesnake
