#pragma once

#include <cstdint>
#include <forward_list>
#include <itlib/small_vector.hpp>
#include <ostream>
#include <string>
#include <string_view>

namespace battlesnake {
namespace rules {

// Standard board sizes.
static constexpr int kBoardSizeSmall = 7;
static constexpr int kBoardSizeMedium = 11;
static constexpr int kBoardSizeLarge = 19;

// Optimize data structures for standard board sizes and numbers of snakes.
// Larger numbers increase memory footprint. Boards with larger size or number
// of snakes cause memory allocations.

// Max board size that memory optimizations will work for.
static constexpr int kOptimizeForMaxBoardSize = kBoardSizeMedium;
// Max number of snakes that memory optimizations will work for.
static constexpr int kOptimizeForMaxSnakesCount = 4;

// All data types use std::string_view instead of strings. It allows to
// eliminate expensive string operations when objects are copied. But it forces
// all strings used to construct the objects to outlive all objects. Use
// StringPool to keep all strings alive.
class StringPool {
 public:
  std::string_view Add(const std::string& s) {
    if (s.empty()) {
      return std::string_view(empty_);
    }
    strings_.push_front(s);
    return std::string_view(strings_.front());
  }

 private:
  std::forward_list<std::string> strings_;
  std::string empty_;
};

using SnakeId = std::string_view;

enum class Move {
  Unknown,  // No move returned from snake.
  Up,
  Down,
  Left,
  Right,
};

struct EliminatedCause {
  enum Cause {
    NotEliminated,
    Collision,
    SelfCollision,
    OutOfHealth,
    HeadToHeadCollision,
    OutOfBounds,
    BySquad,
  };

  Cause cause = NotEliminated;
  SnakeId by_id;
};

struct Point {
  int x;
  int y;

  bool operator==(const Point& other) const {
    return this->x == other.x && this->y == other.y;
  }
  bool operator!=(const Point& other) const { return !operator==(other); }

  Point Up() const { return Point{x, y + 1}; }
  Point Down() const { return Point{x, y - 1}; }
  Point Left() const { return Point{x - 1, y}; }
  Point Right() const { return Point{x + 1, y}; }
  Point Moved(Move move) const;
};

// Max optimized board area is kOptimizeForMaxBoardSize^2, but snakes may have
// extra element at their tail and may go out of bounds, so extra buffer of 2
// elements. This vector should never allocate memory on heap under normal
// conditions, thus improving performance. Though it uses more memory than
// regular std::vector in most cases.
using PointsVector = ::itlib::small_vector<
    Point, kOptimizeForMaxBoardSize * kOptimizeForMaxBoardSize + 2>;

struct PointHash {
  size_t operator()(const Point& point) const {
    size_t x_hash = std::hash<int>()(point.x);
    size_t y_hash = std::hash<int>()(point.y) << 1;
    return x_hash ^ y_hash;
  }
};

struct Snake {
  // Main values used by the engine.
  SnakeId id;
  PointsVector body;
  int health = 0;
  EliminatedCause eliminated_cause;

  // Additional values not necessarily used by ruleset, but used in API.
  std::string_view name;
  std::string_view latency;
  std::string_view shout;
  std::string_view squad;

  bool IsEliminated() const {
    return eliminated_cause.cause != EliminatedCause::NotEliminated;
  }

  bool IsOutOfHealth() const { return health <= 0; }

  Point& Head();
  const Point& Head() const;
  size_t Length() const { return body.size(); }
};

using SnakesVector = ::itlib::small_vector<Snake, kOptimizeForMaxSnakesCount>;

struct BoardState {
  int width = 0;
  int height = 0;
  PointsVector food;
  SnakesVector snakes;
  PointsVector hazards;
};

struct RulesetInfo {
  std::string_view name;
  std::string_view version;
};

struct GameInfo {
  std::string_view id;
  RulesetInfo ruleset;
  int timeout;
};

struct GameState {
  GameInfo game;
  int turn = 0;
  BoardState board;
  Snake you;
};

struct Customization {
  std::string apiversion = "1";
  std::string author;
  std::string color = "#888888";
  std::string head = "default";
  std::string tail = "default";
  std::string version;
};

std::ostream& operator<<(std::ostream& s, Move move);
std::ostream& operator<<(std::ostream& s, EliminatedCause::Cause cause);
std::ostream& operator<<(std::ostream& s, EliminatedCause cause);
std::ostream& operator<<(std::ostream& s, const Point& point);
std::ostream& operator<<(std::ostream& s, const Snake& snake);
std::ostream& operator<<(std::ostream& s, const Snake& snake);
// TODO: implement when needed.
// std::ostream& operator<<(std::ostream& s, const BoardState& state);

}  // namespace rules
}  // namespace battlesnake
