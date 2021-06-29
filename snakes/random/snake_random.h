#include <battlesnake/interface/battlesnake.h>

class SnakeRandom : public battlesnake::interface::Battlesnake {
 public:
  virtual battlesnake::rules::Customization GetCustomization() override;
  virtual void Start(const battlesnake::rules::GameState& game_state) override;
  virtual void End(const battlesnake::rules::GameState& game_state) override;
  virtual MoveResponse Move(
      const battlesnake::rules::GameState& game_state) override;
};
