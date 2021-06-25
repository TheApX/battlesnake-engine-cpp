#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "battlesnake/interface/battlesnake.h"

namespace battlesnake {
namespace cli {

class HttpClientBattlesnake : public battlesnake::interface::Battlesnake {
 public:
  HttpClientBattlesnake(const std::string& url);
  ~HttpClientBattlesnake();

  virtual battlesnake::rules::Customization GetCustomization() override;
  virtual void Begin(const battlesnake::rules::GameState& game_state) override;
  virtual void End(const battlesnake::rules::GameState& game_state) override;
  virtual battlesnake::rules::Move Move(
      const battlesnake::rules::GameState& game_state) override;

 private:
  std::string url_;
};

}  // namespace cli
}  // namespace battlesnake
