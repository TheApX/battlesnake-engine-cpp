#include "battlesnake/rules/squad_ruleset.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "battlesnake/rules/errors.h"

namespace battlesnake {
namespace rules {

BoardState SquadRuleset::CreateNextBoardState(const BoardState& prev_state,
                                              std::map<SnakeId, Move> moves,
                                              int turn) {
  BoardState next_state =
      StandardRuleset::CreateNextBoardState(prev_state, moves, turn);

  resurrectSquadBodyCollisions(next_state);
  shareSquadAttributes(next_state);

  return next_state;
}

void SquadRuleset::resurrectSquadBodyCollisions(BoardState& state) const {
  if (!squad_config_.allow_body_collisions) {
    return;
  }

  std::unordered_map<SnakeId, std::string> snake_squads;
  for (const Snake& snake : state.snakes) {
    snake_squads[snake.id] = snake.squad;
  }

  for (Snake& snake : state.snakes) {
    if (snake.eliminated_cause.cause != EliminatedCause::Collision) {
      continue;
    }

    auto eliminator_squad = snake_squads.find(snake.eliminated_cause.by_id);
    if (eliminator_squad == snake_squads.end()) {
      throw ErrorInvalidEliminatedById(snake.id, snake.eliminated_cause.by_id);
    }

    if (snake.squad != eliminator_squad->second) {
      // Legit elimination.
      continue;
    }

    snake.eliminated_cause.cause = EliminatedCause::NotEliminated;
    snake.eliminated_cause.by_id = "";
  }
}

void SquadRuleset::shareSquadAttributes(BoardState& state) const {
  if (!squad_config_.shared_elimination && !squad_config_.shared_health &&
      !squad_config_.shared_length) {
    return;
  }

  for (Snake& snake : state.snakes) {
    for (Snake& other_snake : state.snakes) {
      if (snake.squad != other_snake.squad) {
        continue;
      }

      if (squad_config_.shared_health) {
        snake.health = std::max(snake.health, other_snake.health);
      }

      if (squad_config_.shared_length) {
        while (snake.Length() < other_snake.Length()) {
          growSnake(snake);
        }
      }

      if (squad_config_.shared_elimination) {
        if (!snake.IsEliminated() && other_snake.IsEliminated()) {
          snake.eliminated_cause.cause = EliminatedCause::BySquad;
          snake.eliminated_cause.by_id = "";
        }
      }
    }
  }
}

bool SquadRuleset::IsGameOver(const BoardState& state) {
  std::unordered_set<std::string> squads_not_eliminated;

  for (const Snake& snake : state.snakes) {
    if (!snake.IsEliminated()) {
      squads_not_eliminated.insert(snake.squad);
    }
  }

  return squads_not_eliminated.size() <= 1;
}

}  // namespace rules
}  // namespace battlesnake
