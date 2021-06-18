#include "ruleset.h"

namespace battlesnake {
namespace engine {

class StandardRuleset : public Ruleset {
 public:
  // Default values.
  static constexpr int kDefaultFoodSpawnChance = 15;
  static constexpr int kDefaultMinimumFood = 1;
  static constexpr int kDefaultSnakeMaxHealth = 100;
  static constexpr int kDefaultSnakeStartSize = 3;

  StandardRuleset(int food_spawn_chance = kDefaultFoodSpawnChance,
                  int minimum_food = kDefaultMinimumFood,
                  int snake_max_health = kDefaultSnakeMaxHealth,
                  int snake_start_size = kDefaultSnakeStartSize)
      : food_spawn_chance_(food_spawn_chance),
        minimum_food_(minimum_food),
        snake_max_health_(snake_max_health),
        snake_start_size_(snake_start_size)
        {}

  virtual BoardState CreateInitialBoardState(int width, int height, std::vector<SnakeId> snakeIDs) override;
  virtual BoardState CreateNextBoardState(const BoardState& prev_state, std::map<SnakeId, Move> moves) override;
  virtual bool IsGameOver(const BoardState& state) override;

 private:
  int food_spawn_chance_ = 0;  // [0, 100]
  int minimum_food_ = 0;
  int snake_max_health_ = 0;
  int snake_start_size_ = 0;
};

}  // namespace engine
}  // namespace battlesnake
