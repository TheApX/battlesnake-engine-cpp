#pragma once

#include "battlesnake/rules/standard_ruleset.h"

namespace battlesnake {
namespace rules {

class SoloRuleset : public StandardRuleset {
 public:
  SoloRuleset(const Config& config = Config::Default())
      : StandardRuleset(config) {}

  virtual bool IsGameOver(const BoardState& state) override;
};

}  // namespace rules
}  // namespace battlesnake
