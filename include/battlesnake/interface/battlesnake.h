#pragma once

#include "battlesnake/rules/data_types.h"

namespace battlesnake {
namespace interface {

// Public battlesnake interface. The engine doesn't validate timeout, it must be
// handled by the implementation. For example, an implementation that sends HTTP
// requests must send it with timeout.
class Battlesnake {
 public:
  virtual ~Battlesnake(){};

  virtual battlesnake::rules::Customization GetCustomization() {
    return battlesnake::rules::Customization();
  };

  virtual void Begin(const battlesnake::rules::GameState& game_state){};

  virtual void End(const battlesnake::rules::GameState& game_state){};

  virtual battlesnake::rules::Move Move(
      const battlesnake::rules::GameState& game_state) {
    return battlesnake::rules::Move::Unknown;
  };
};

}  // namespace interface
}  // namespace battlesnake
