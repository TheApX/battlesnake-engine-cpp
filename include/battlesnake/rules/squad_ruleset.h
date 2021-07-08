#pragma once

#include "battlesnake/rules/standard_ruleset.h"

namespace battlesnake {
namespace rules {

class SquadRuleset : public StandardRuleset {
 public:
  struct SquadConfig {
    bool allow_body_collisions = true;
    bool shared_elimination = true;
    bool shared_health = true;
    bool shared_length = true;

    static SquadConfig Default() { return SquadConfig(); }
  };

  SquadRuleset(const Config& config = Config::Default(),
               const SquadConfig& squad_config = SquadConfig::Default())
      : StandardRuleset(config), squad_config_(squad_config) {}

  virtual BoardState CreateNextBoardState(
      const BoardState& prev_state,
      const std::unordered_map<SnakeId, Move>& moves, int turn) override;
  virtual bool IsGameOver(const BoardState& state) override;

 private:
  SquadConfig squad_config_;

  void resurrectSquadBodyCollisions(BoardState& state) const;
  void shareSquadAttributes(BoardState& state) const;
};

}  // namespace rules
}  // namespace battlesnake
