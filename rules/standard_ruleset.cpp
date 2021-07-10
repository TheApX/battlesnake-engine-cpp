#include "battlesnake/rules/standard_ruleset.h"

#include <algorithm>
#include <random>
#include <unordered_set>
#include <vector>

#include "battlesnake/rules/errors.h"

namespace battlesnake {
namespace rules {

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
        .health = config_.snake_max_health,
    });
  }

  if (isKnownBoardSize(initial_board_state)) {
    placeSnakesFixed(initial_board_state);
    placeFoodFixed(initial_board_state);
  } else {
    PointsVector unoccupied_points =
        getEvenUnoccupiedPoints(initial_board_state);
    placeSnakesRandomly(initial_board_state, unoccupied_points);
    placeFoodRandomly(initial_board_state, unoccupied_points);
  }

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
    state.snakes[i].body.reserve(config_.snake_start_size);
    for (int j = 0; j < config_.snake_start_size; ++j) {
      state.snakes[i].body.push_back(start_points[i]);
    }
  }
}

void StandardRuleset::placeSnakesRandomly(
    BoardState& state, PointsVector& unoccupied_points) const {
  for (Snake& snake : state.snakes) {
    if (unoccupied_points.empty()) {
      throw ErrorNoRoomForSnake();
    }

    int ri = getRandomNumber(unoccupied_points.size());
    const Point& p = unoccupied_points[ri];
    snake.body.reserve(config_.snake_start_size);
    for (int j = 0; j < config_.snake_start_size; ++j) {
      snake.body.push_back(p);
    }

    if (unoccupied_points.size() == 1) {
      unoccupied_points.clear();
    } else {
      unoccupied_points[ri] =
          std::move(unoccupied_points[unoccupied_points.size() - 1]);
      unoccupied_points.resize(unoccupied_points.size() - 1);
    }
  }
}

void StandardRuleset::placeFoodFixed(BoardState& state) const {
  // Place 1 food within exactly 2 moves of each snake.
  std::unordered_set<Point, PointHash> food_locations;

  for (const Snake& snake : state.snakes) {
    const Point& snake_head = snake.body.front();
    std::initializer_list<Point> possible_food_locations{
        Point{.x = snake_head.x - 1, .y = snake_head.y - 1},
        Point{.x = snake_head.x - 1, .y = snake_head.y + 1},
        Point{.x = snake_head.x + 1, .y = snake_head.y - 1},
        Point{.x = snake_head.x + 1, .y = snake_head.y + 1},
    };

    PointsVector available_food_locations;
    for (const Point& p : possible_food_locations) {
      if (food_locations.find(p) != food_locations.end()) {
        // Already occupied by a food.
        continue;
      }
      available_food_locations.push_back(p);
    }

    if (available_food_locations.size() <= 0) {
      throw ErrorNoRoomForFood();
    }

    const Point& placed_food = available_food_locations[getRandomNumber(
        available_food_locations.size())];
    state.food.push_back(placed_food);
    food_locations.insert(placed_food);
  }

  // Always place 1 food in center of board for dramatic purposes.
  Point center_coordinates{(state.width - 1) / 2, (state.height - 1) / 2};
  if (food_locations.find(center_coordinates) != food_locations.end()) {
    throw ErrorNoRoomForFood();
  }
  state.food.push_back(center_coordinates);
}

void StandardRuleset::placeFoodRandomly(BoardState& state,
                                        PointsVector& unoccupied_points) const {
  spawnFood(state, state.snakes.size(), unoccupied_points);
}

void StandardRuleset::maybeSpawnFood(BoardState& state) const {
  int num_current_food = state.food.size();
  if (num_current_food < config_.minimum_food) {
    auto unoccupied_points = getUnoccupiedPoints(state, false);
    spawnFood(state, config_.minimum_food - num_current_food,
              unoccupied_points);
    return;
  }
  if (config_.food_spawn_chance > 0 &&
      getRandomNumber(100) < config_.food_spawn_chance) {
    auto unoccupied_points = getUnoccupiedPoints(state, false);
    spawnFood(state, 1, unoccupied_points);
    return;
  }
}

void StandardRuleset::spawnFood(BoardState& state, int count,
                                PointsVector& unoccupied_points) const {
  for (int i = 0; i < count; ++i) {
    if (unoccupied_points.empty()) {
      return;
    }
    int ri = getRandomNumber(unoccupied_points.size());
    state.food.push_back(unoccupied_points[ri]);

    if (unoccupied_points.size() == 1) {
      unoccupied_points.clear();
    } else {
      unoccupied_points[ri] =
          std::move(unoccupied_points[unoccupied_points.size() - 1]);
      unoccupied_points.resize(unoccupied_points.size() - 1);
    }
  }
}

PointsVector StandardRuleset::getUnoccupiedPoints(
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

  PointsVector unoccupied_points;
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

PointsVector StandardRuleset::getEvenUnoccupiedPoints(const BoardState& state) {
  return getUnoccupiedPoints(
      state, false, [](const Point& p) { return (p.x + p.y) % 2 == 0; });
}

BoardState StandardRuleset::CreateNextBoardState(
    const BoardState& prev_state,
    const std::unordered_map<SnakeId, Move>& moves, int turn) {
  BoardState next_state = prev_state;

  moveSnakes(next_state, moves);
  reduceSnakeHealth(next_state);
  maybeFeedSnakes(next_state);
  maybeSpawnFood(next_state);
  maybeEliminateSnakes(next_state);

  return next_state;
}

void StandardRuleset::moveSnakes(
    BoardState& state, const std::unordered_map<SnakeId, Move>& moves) const {
  checkSnakesForMove(state, moves);

  for (Snake& snake : state.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }
    auto move_it = moves.find(snake.id);
    if (move_it == moves.end()) {
      throw ErrorNoMoveFound(std::string(snake.id));
    }
    Move move = move_it->second;

    Point old_head = snake.body.front();
    // Default direction is Up.
    Point new_head = old_head.Up();

    switch (move) {
      case Move::Up:
        new_head = old_head.Up();
        break;
      case Move::Down:
        new_head = old_head.Down();
        break;
      case Move::Left:
        new_head = old_head.Left();
        break;
      case Move::Right:
        new_head = old_head.Right();
        break;

      default:
        // If neck is available, use neck to determine last direction.
        if (snake.body.size() < 2) {
          break;
        }
        const Point& neck = snake.body[1];
        if (old_head == neck.Up()) {
          new_head = old_head.Up();
        }
        if (old_head == neck.Down()) {
          new_head = old_head.Down();
        }
        if (old_head == neck.Left()) {
          new_head = old_head.Left();
        }
        if (old_head == neck.Right()) {
          new_head = old_head.Right();
        }
        break;
    }

    // Shift body to tail.
    for (int i = snake.body.size() - 1; i > 0; --i) {
      snake.body[i] = snake.body[i - 1];
    }
    // Set new head.
    snake.body.front() = new_head;
  }
}

void StandardRuleset::checkSnakesForMove(
    BoardState& state, const std::unordered_map<SnakeId, Move>& moves) const {
  // Check that all non-eliminated snakes have moves and bodies.
  for (const Snake& snake : state.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }

    if (snake.body.empty()) {
      throw ErrorZeroLengthSnake(std::string(snake.id));
    }

    if (moves.find(snake.id) == moves.end()) {
      throw ErrorNoMoveFound(std::string(snake.id));
    }
  }
}

void StandardRuleset::reduceSnakeHealth(BoardState& state) const {
  for (Snake& snake : state.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }
    snake.health--;
  }
}

void StandardRuleset::maybeFeedSnakes(BoardState& state) const {
  for (int i = 0; i < state.food.size(); ++i) {
    const Point& food = state.food[i];
    bool food_has_been_eaten = false;
    for (Snake& snake : state.snakes) {
      if (snake.IsEliminated() || snake.body.size() == 0) {
        continue;
      }
      const Point& head = snake.body.front();
      if (head == food) {
        feedSnake(snake);
        food_has_been_eaten = true;
      }
    }

    if (food_has_been_eaten) {
      state.food[i] = std::move(state.food[state.food.size() - 1]);
      state.food.resize(state.food.size() - 1);
      --i;
    }
  }
}

void StandardRuleset::feedSnake(Snake& snake) const {
  growSnake(snake);
  snake.health = config_.snake_max_health;
}

void StandardRuleset::growSnake(Snake& snake) const {
  if (snake.body.empty()) {
    return;
  }
  snake.body.push_back(snake.body[snake.body.size() - 1]);
}

void StandardRuleset::maybeEliminateSnakes(BoardState& state) const {
  SnakeIndicesVector snake_indices_by_length;
  snake_indices_by_length.reserve(state.snakes.size());
  for (int i = 0; i < state.snakes.size(); ++i) {
    snake_indices_by_length.push_back(i);
  }
  std::sort(snake_indices_by_length.begin(), snake_indices_by_length.end(),
            [&](int a, int b) -> bool {
              int len_a = state.snakes[a].body.size();
              int len_b = state.snakes[b].body.size();
              return len_a > len_b;
            });

  // First, iterate over all non-eliminated snakes and eliminate the ones
  // that are out of health or have moved out of bounds.
  eliminateOutOfHealthOrBoundsSnakes(state);

  std::unordered_map<SnakeId, EliminatedCause> collision_eliminations =
      findCollisionEliminations(state, snake_indices_by_length);
  applyCollisionEliminations(state, collision_eliminations);
}

void StandardRuleset::eliminateOutOfHealthOrBoundsSnakes(
    BoardState& state) const {
  for (Snake& snake : state.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }

    if (snake.body.empty()) {
      throw ErrorZeroLengthSnake(std::string(snake.id));
    }

    if (snake.IsOutOfHealth()) {
      snake.eliminated_cause.cause = EliminatedCause::OutOfHealth;
      continue;
    }

    if (snakeOutOfBounds(state, snake)) {
      snake.eliminated_cause.cause = EliminatedCause::OutOfBounds;
      continue;
    }
  }
}

bool StandardRuleset::snakeOutOfBounds(const BoardState& state,
                                       const Snake& snake) const {
  for (const Point& p : snake.body) {
    if (p.x < 0 || p.x >= state.width) {
      return true;
    }
    if (p.y < 0 || p.y >= state.height) {
      return true;
    }
  }
  return false;
}

std::unordered_map<SnakeId, EliminatedCause>
StandardRuleset::findCollisionEliminations(
    const BoardState& state,
    const SnakeIndicesVector& snake_indices_by_length) const {
  std::unordered_map<SnakeId, EliminatedCause> result;
  for (const Snake& snake : state.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }

    // Check for self-collision first.
    if (snakeHasBodyCollided(snake, snake)) {
      result[snake.id] = EliminatedCause{
          .cause = EliminatedCause::SelfCollision,
          .by_id = config_.optimize_strings ? std::string() : snake.id,
      };
      continue;
    }

    // Check for body collisions with other snakes.
    {
      bool has_body_collided = false;
      for (int i = 0; i < snake_indices_by_length.size(); ++i) {
        const Snake& other = state.snakes[snake_indices_by_length[i]];
        if (other.IsEliminated()) {
          continue;
        }
        if (snake.id == other.id) {
          continue;
        }
        if (snakeHasBodyCollided(snake, other)) {
          result[snake.id] = EliminatedCause{
              .cause = EliminatedCause::Collision,
              .by_id = other.id,
          };
          has_body_collided = true;
          break;
        }
      }
      if (has_body_collided) {
        continue;
      }
    }

    // Check for head-to-head.
    {
      bool has_head_collided = false;
      for (int i = 0; i < snake_indices_by_length.size(); ++i) {
        const Snake& other = state.snakes[snake_indices_by_length[i]];
        if (other.IsEliminated()) {
          continue;
        }
        if (snake.id == other.id) {
          continue;
        }
        if (snakeHasLostHeadToHead(snake, other)) {
          result[snake.id] = EliminatedCause{
              .cause = EliminatedCause::HeadToHeadCollision,
              .by_id = other.id,
          };
          has_head_collided = true;
          break;
        }
      }
      if (has_head_collided) {
        continue;
      }
    }
  }

  return result;
}

bool StandardRuleset::snakeHasBodyCollided(const Snake& snake,
                                           const Snake& other) const {
  const Point& head = snake.Head();
  for (int i = 1; i < other.body.size(); ++i) {
    if (head == other.body[i]) {
      return true;
    }
  }
  return false;
}

bool StandardRuleset::snakeHasLostHeadToHead(const Snake& snake,
                                             const Snake& other) const {
  if (snake.Head() != other.Head()) {
    return false;
  }

  return snake.Length() <= other.Length();
}

void StandardRuleset::applyCollisionEliminations(
    BoardState& state,
    const std::unordered_map<SnakeId, EliminatedCause>& eliminations) const {
  for (Snake& snake : state.snakes) {
    auto elimination = eliminations.find(snake.id);
    if (elimination == eliminations.end()) {
      continue;
    }
    snake.eliminated_cause = elimination->second;
  }
}

bool StandardRuleset::IsGameOver(const BoardState& state) {
  int num_snakes_remaining = 0;
  for (const Snake& snake : state.snakes) {
    if (!snake.IsEliminated()) {
      num_snakes_remaining++;
    }
  }

  return num_snakes_remaining <= 1;
}

}  // namespace rules
}  // namespace battlesnake
