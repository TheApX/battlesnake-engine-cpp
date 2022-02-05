#include "battlesnake/rules/wrapped_ruleset.h"

namespace battlesnake {
namespace rules {

WrappedRuleset::RoyaleConfig WrappedRuleset::fixRoyaleConfig(
    const RoyaleConfig& royale_config) {
  RoyaleConfig result = royale_config;

  // Do not shrink at all.
  result.shrink_every_n_turns = 1000000;

  return result;
}

}  // namespace rules
}  // namespace battlesnake
