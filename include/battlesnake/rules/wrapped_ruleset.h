#pragma once

#include "battlesnake/rules/royale_ruleset.h"

namespace battlesnake {
namespace rules {

class WrappedRuleset : public RoyaleRuleset {
 public:
  WrappedRuleset(const Config& config = Config::Default(),
                 const RoyaleConfig& royale_config = RoyaleConfig::Default())
      : RoyaleRuleset(config, fixRoyaleConfig(royale_config)) {
    wrapped_mode_ = true;
  }

  virtual void CreateNextBoardState(const BoardState& prev_state,
                                    const SnakeMovesVector& moves, int turn,
                                    BoardState& next_state) override;

 private:
  static RoyaleConfig fixRoyaleConfig(const RoyaleConfig& royale_config);

  void updateWrappedHazard(BoardState& next_state, int turn);
  void generateHazardPattern(int hazard_num);
};

}  // namespace rules
}  // namespace battlesnake
