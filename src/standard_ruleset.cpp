#include "standard_ruleset.h"

#include <algorithm>
#include <random>
#include <unordered_set>

#include "ruleset_errors.h"

namespace battlesnake {
namespace engine {

BoardState StandardRuleset::CreateInitialBoardState(
    int width, int height, std::vector<SnakeId> snake_ids) {
  BoardState initial_board_state{
      .width = width,
      .height = height,
  };

  initial_board_state.snakes.reserve(snake_ids.size());
  for (const SnakeId snake_id : snake_ids) {
    initial_board_state.snakes.push_back(Snake{
        .id = snake_id,
        .health = snake_max_health_,
    });
  }
  placeSnakes(initial_board_state);

  return initial_board_state;
}

int StandardRuleset::getRandomNumber(int max_value) {
  std::random_device random_device;
  std::mt19937 generator(random_device());
  std::uniform_int_distribution<> distribution(0, max_value - 1);

  return distribution(generator);
}

bool StandardRuleset::isKnownBoardSize(const BoardState& state) {
  if (state.width == kBoardSizeSmall && state.height == kBoardSizeSmall) {
    return true;
  }
  if (state.width == kBoardSizeMedium && state.height == kBoardSizeMedium) {
    return true;
  }
  if (state.width == kBoardSizeLarge && state.height == kBoardSizeLarge) {
    return true;
  }
  return false;
}

void StandardRuleset::placeSnakes(BoardState& state) const {
  if (isKnownBoardSize(state)) {
    placeSnakesFixed(state);
  } else {
    placeSnakesRandomly(state);
  }
}

void StandardRuleset::placeSnakesFixed(BoardState& state) const {
  int pos_left = 1;
  int pos_mid = (state.width - 1) / 2;
  int pos_right = state.width - 2;

  std::vector<Point> start_points{
      {pos_left, pos_left},  {pos_left, pos_mid},   {pos_left, pos_right},  //
      {pos_mid, pos_left},   {pos_left, pos_right},                         //
      {pos_right, pos_left}, {pos_right, pos_mid},  {pos_right, pos_right},
  };

  // Check that there is enough space for all snakes.
  if (state.snakes.size() > start_points.size()) {
    throw ErrorTooManySnakes(state.snakes.size());
  }
  // Reorder starting positions randomly.
  std::random_shuffle(start_points.begin(), start_points.end());

  // Assign snakes in the given order.
  for (size_t i = 0; i < state.snakes.size(); ++i) {
    state.snakes[i].body.reserve(snake_start_size_);
    for (int j = 0; j < snake_start_size_; ++j) {
      state.snakes[i].body.push_back(start_points[i]);
    }
  }
}

void StandardRuleset::placeSnakesRandomly(BoardState& state) const {
  std::vector<Point> unoccupied_points = getEvenUnoccupiedPoints(state);

  for (Snake& snake : state.snakes) {
    if (unoccupied_points.empty()) {
      throw ErrorNoRoomForSnake();
    }

    int ri = getRandomNumber(unoccupied_points.size());
    const Point& p = unoccupied_points[ri];
    snake.body.reserve(snake_start_size_);
    for (int j = 0; j < snake_start_size_; ++j) {
      snake.body.push_back(p);
    }

    if (unoccupied_points.size() == 1) {
      unoccupied_points.clear();
    } else {
      unoccupied_points[ri] = unoccupied_points[unoccupied_points.size() - 1];
      unoccupied_points.resize(unoccupied_points.size() - 1);
    }
  }
}

std::vector<Point> StandardRuleset::getUnoccupiedPoints(
    const BoardState& state, bool include_possible_moves,
    const std::function<bool(const Point&)>& filter) {
  std::unordered_set<Point, PointHash> occupied_points;

  for (const Snake& snake : state.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }

    for (const Point& p : snake.body) {
      occupied_points.insert(p);
    }

    if (include_possible_moves && snake.body.size() > 0) {
      const Point& head = snake.body.front();
      occupied_points.insert(head.Up());
      occupied_points.insert(head.Down());
      occupied_points.insert(head.Left());
      occupied_points.insert(head.Right());
    }
  }

  std::vector<Point> unoccupied_points;
  for (int y = 0; y < state.height; ++y) {
    for (int x = 0; x < state.width; ++x) {
      Point p{x, y};
      if (occupied_points.find(p) != occupied_points.end()) {
        continue;
      }
      if (!filter(p)) {
        continue;
      }
      unoccupied_points.push_back(p);
    }
  }

  return unoccupied_points;
}

std::vector<Point> StandardRuleset::getEvenUnoccupiedPoints(
    const BoardState& state) {
  return getUnoccupiedPoints(
      state, false, [](const Point& p) { return (p.x + p.y) % 2 == 0; });
}

BoardState StandardRuleset::CreateNextBoardState(
    const BoardState& prev_state, std::map<SnakeId, Move> moves) {
  BoardState next_state;

  return next_state;
}

bool StandardRuleset::IsGameOver(const BoardState& state) { return true; }

}  // namespace engine
}  // namespace battlesnake
