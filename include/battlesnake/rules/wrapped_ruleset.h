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

 private:
  static RoyaleConfig fixRoyaleConfig(const RoyaleConfig& royale_config);
};

}  // namespace rules
}  // namespace battlesnake
