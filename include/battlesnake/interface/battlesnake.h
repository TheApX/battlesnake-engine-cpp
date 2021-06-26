#pragma once

#include "battlesnake/rules/data_types.h"

namespace battlesnake {
namespace interface {

// Public battlesnake interface. The engine doesn't validate timeout, it must be
// handled by the implementation. For example, an implementation that sends HTTP
// requests must send it with timeout.
class Battlesnake {
 public:
  // Response to the Move call.
  struct MoveResponse {
    battlesnake::rules::Move move = battlesnake::rules::Move::Unknown;
    std::string shout;
  };

  virtual ~Battlesnake(){};

  virtual battlesnake::rules::Customization GetCustomization() {
    return battlesnake::rules::Customization();
  };

  virtual void Start(const battlesnake::rules::GameState& game_state){};

  virtual void End(const battlesnake::rules::GameState& game_state){};

  virtual MoveResponse Move(const battlesnake::rules::GameState& game_state) {
    return MoveResponse();
  };
};

}  // namespace interface
}  // namespace battlesnake
