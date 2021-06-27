#pragma once

#include <functional>

#include "battlesnake/rules/ruleset.h"

namespace battlesnake {
namespace rules {

class StandardRuleset : public Ruleset {
 public:
  // Default values.
  struct Config {
    int food_spawn_chance = 15;  // [0, 100]
    int minimum_food = 1;
    int snake_max_health = 100;
    int snake_start_size = 3;

    static Config Default() { return Config(); }
  };

  StandardRuleset(const Config& config = Config::Default()) : config_(config) {}

  virtual BoardState CreateInitialBoardState(
      int width, int height, std::vector<SnakeId> snake_ids) override;
  virtual BoardState CreateNextBoardState(const BoardState& prev_state,
                                          std::map<SnakeId, Move> moves,
                                          int turn = 0) override;
  virtual bool IsGameOver(const BoardState& state) override;

 protected:
  static int getRandomNumber(int max_value);

 private:
  Config config_;

  static bool isKnownBoardSize(const BoardState& state);

  void placeSnakesFixed(BoardState& state) const;
  void placeSnakesRandomly(BoardState& state,
                           std::vector<Point>& unoccupied_points) const;

  void placeFoodFixed(BoardState& state) const;
  void placeFoodRandomly(BoardState& state,
                         std::vector<Point>& unoccupied_points) const;
  void maybeSpawnFood(BoardState& state) const;
  void spawnFood(BoardState& state, int count,
                 std::vector<Point>& unoccupied_points) const;

  static std::vector<Point> getUnoccupiedPoints(
      const BoardState& state, bool include_possible_moves,
      const std::function<bool(const Point&)>& filter = [](const Point&) {
        return true;
      });
  static std::vector<Point> getEvenUnoccupiedPoints(const BoardState& state);

  void moveSnakes(BoardState& state, std::map<SnakeId, Move> moves) const;
  void checkSnakesForMove(BoardState& state,
                          std::map<SnakeId, Move> moves) const;

  void reduceSnakeHealth(BoardState& state) const;

  void maybeFeedSnakes(BoardState& state) const;
  void feedSnake(Snake& snake) const;
  void growSnake(Snake& snake) const;

  void maybeEliminateSnakes(BoardState& state) const;
  void eliminateOutOfHealthOrBoundsSnakes(BoardState& state) const;
  bool snakeOutOfBounds(const BoardState& state, const Snake& snake) const;
  std::map<SnakeId, EliminatedCause> findCollisionEliminations(
      const BoardState& state,
      const std::vector<int>& snake_indices_by_length) const;
  bool snakeHasBodyCollided(const Snake& snake, const Snake& other) const;
  bool snakeHasLostHeadToHead(const Snake& snake, const Snake& other) const;
  void applyCollisionEliminations(
      BoardState& state,
      const std::map<SnakeId, EliminatedCause>& eliminations) const;
};

}  // namespace rules
}  // namespace battlesnake
