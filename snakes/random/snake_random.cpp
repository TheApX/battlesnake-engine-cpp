#include "snake_random.h"

#include <iostream>
#include <random>
#include <vector>

using namespace battlesnake::rules;
using namespace battlesnake::interface;

Customization SnakeRandom::GetCustomization() {
  std::cout << "Customization" << std::endl;

  return Customization{
      .color = "#de2ac9",
      .head = "rudolph",
      .tail = "bonhomme",
  };
};

void SnakeRandom::Start(const GameState& game_state) {
  std::cout << "Start: " << game_state.game.id << std::endl;
};

void SnakeRandom::End(const GameState& game_state) {
  std::cout << "End: " << game_state.game.id << std::endl;
};

Battlesnake::MoveResponse SnakeRandom::Move(const GameState& game_state) {
  std::vector<battlesnake::rules::Move> possible_moves{
      Move::Left,
      Move::Right,
      Move::Up,
      Move::Down,
  };

  std::random_device random_device;
  std::mt19937 generator(random_device());
  std::uniform_int_distribution<> distribution(0, possible_moves.size() - 1);

  Battlesnake::MoveResponse result{
      .move = possible_moves[distribution(generator)],
  };

  std::cout << "Move: " << game_state.game.id << " turn " << game_state.turn
            << "  -  " << result.move << std::endl;
  return result;
};
