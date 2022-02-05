#pragma once

#include "battlesnake/rules/standard_ruleset.h"

namespace battlesnake {
namespace rules {

class WrappedRuleset : public StandardRuleset {
 public:
  WrappedRuleset(const Config& config = Config::Default())
      : StandardRuleset(config) {
    wrapped_mode_ = true;
  }
};

}  // namespace rules
}  // namespace battlesnake
