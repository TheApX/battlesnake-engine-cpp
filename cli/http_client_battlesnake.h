#pragma once

#include <string>

#include "battlesnake/interface/battlesnake.h"

namespace battlesnake {
namespace cli {

class HttpClientBattlesnake : public battlesnake::interface::Battlesnake {
 public:
  HttpClientBattlesnake(const std::string& url);
  ~HttpClientBattlesnake();

  virtual battlesnake::rules::Customization GetCustomization() override;
  virtual void Start(const battlesnake::rules::GameState& game_state) override;
  virtual void End(const battlesnake::rules::GameState& game_state) override;
  virtual MoveResponse Move(
      const battlesnake::rules::GameState& game_state) override;

 private:
  std::string url_;
};

}  // namespace cli
}  // namespace battlesnake
