#pragma once

#include "battlesnake/rules/standard_ruleset.h"

namespace battlesnake {
namespace rules {

class ConstrictorRuleset : public StandardRuleset {
 public:
  ConstrictorRuleset(const Config& config = Config::Default())
      : StandardRuleset(config), snake_max_health_(config.snake_max_health) {}

  virtual BoardState CreateInitialBoardState(
      Coordinate width, Coordinate height,
      std::vector<SnakeId> snake_ids) override;
  virtual void CreateNextBoardState(const BoardState& prev_state,
                                    const SnakeMovesVector& moves, int turn,
                                    BoardState& next_state) override;

 private:
  int snake_max_health_ = 0;

  void applyConstrictorRules(BoardState& state) const;
};

}  // namespace rules
}  // namespace battlesnake
