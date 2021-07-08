#pragma once

#include <functional>

#include "battlesnake/rules/standard_ruleset.h"

namespace battlesnake {
namespace rules {

class RoyaleRuleset : public StandardRuleset {
 public:
  struct RoyaleConfig {
    int shrink_every_n_turns = 25;
    int extra_damage_per_turn = 15;

    static RoyaleConfig Default() { return RoyaleConfig(); }
  };

  RoyaleRuleset(const Config& config = Config::Default(),
                const RoyaleConfig& royale_config = RoyaleConfig::Default())
      : StandardRuleset(config), royale_config_(royale_config) {}

  virtual BoardState CreateNextBoardState(
      const BoardState& prev_state,
      const std::unordered_map<SnakeId, Move>& moves, int turn) override;

 private:
  RoyaleConfig royale_config_;

  struct Bounds {
    int min_x = 0;
    int max_x = 0;
    int min_y = 0;
    int max_y = 0;
  };

  Bounds findBounds(const BoardState& state) const;
  void damageOutOfBounds(const Bounds& bounds, BoardState& state) const;
  bool maybeShrinkBounds(int turn, Bounds& bounds) const;
  void fillHazards(const Bounds& bounds, BoardState& state) const;
};

}  // namespace rules
}  // namespace battlesnake
