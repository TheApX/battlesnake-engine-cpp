#pragma once

#include <functional>
#include <memory>

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

  // There are two interfaces. Simple one just returns the result. All
  // computations and cleanup is finished before responding. "Async" interface
  // allows to respond before returning via callback. It allows for additional
  // code (such as cleanup) executed after responding to client.

  // Simple interface. Override these functions if you don't need to do anything
  // after responding.

  virtual battlesnake::rules::Customization GetCustomization() {
    return battlesnake::rules::Customization();
  };
  virtual void Start(const battlesnake::rules::GameState& game_state){};
  virtual void End(const battlesnake::rules::GameState& game_state){};
  virtual MoveResponse Move(const battlesnake::rules::GameState& game_state) {
    return MoveResponse();
  };

  // "Async" interface. Override these functions if you need to do something
  // extra after responding. `game_state` uses references to strings in the
  // string pool. Keep `string_pool` alive while `game_state` is used.

  virtual void GetCustomization(
      std::function<void(const battlesnake::rules::Customization& result)>
          respond);
  virtual void Start(
      std::shared_ptr<battlesnake::rules::StringPool> string_pool,
      const battlesnake::rules::GameState& game_state,
      std::function<void()> respond);
  virtual void End(std::shared_ptr<battlesnake::rules::StringPool> string_pool,
                   const battlesnake::rules::GameState& game_state,
                   std::function<void()> respond);
  virtual void Move(std::shared_ptr<battlesnake::rules::StringPool> string_pool,
                    const battlesnake::rules::GameState& game_state,
                    std::function<void(const MoveResponse& result)> respond);
};

}  // namespace interface
}  // namespace battlesnake
