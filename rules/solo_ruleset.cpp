#include "battlesnake/rules/solo_ruleset.h"

namespace battlesnake {
namespace rules {

bool SoloRuleset::IsGameOver(const BoardState& state) {
  for (const Snake& snake : state.snakes) {
    if (!snake.IsEliminated()) {
      return false;
    }
  }

  return true;
}

}  // namespace rules
}  // namespace battlesnake
