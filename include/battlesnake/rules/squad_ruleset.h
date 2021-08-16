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

  virtual void CreateNextBoardState(const BoardState& prev_state,
                                    const SnakeMovesVector& moves, int turn,
                                    BoardState& next_state) override;
  virtual bool IsGameOver(const BoardState& state) override;

  using Ruleset::CreateNextBoardState;

 private:
  SquadConfig squad_config_;

  void resurrectSquadBodyCollisions(BoardState& state) const;
  void shareSquadAttributes(BoardState& state) const;
};

}  // namespace rules
}  // namespace battlesnake
