#pragma once

#include <cstdint>
#include <forward_list>
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>
#include <trivial_loop_array.hpp>
#include <unordered_map>

namespace battlesnake {
namespace rules {

using Coordinate = signed char;

// Standard board sizes.
static constexpr Coordinate kBoardSizeSmall = 7;
static constexpr Coordinate kBoardSizeMedium = 11;
static constexpr Coordinate kBoardSizeLarge = 19;
static constexpr Coordinate kBoardSizeMax = 25;

// Standard and max snakes count.
static constexpr Coordinate kSnakesCountStandard = 4;
static constexpr Coordinate kSnakesCountDuel = 2;
static constexpr Coordinate kSnakesCountMax = 8;

// Optimize data structures for standard board sizes and numbers of snakes.
// Larger numbers increase memory footprint. Boards with larger size or number
// of snakes cause memory allocations.

// Max board size that memory optimizations will work for.
static constexpr int kOptimizeForMaxBoardSize = kBoardSizeMedium;
// Max number of snakes that memory optimizations will work for.
static constexpr int kOptimizeForMaxSnakesCount = kSnakesCountStandard;

// Wrapper for string pointer. Can be used as simpler but trivially
// constructible, destructible and copyable alternative for std::string_view.
struct StringWrapper {
 public:
  std::string* value;

  const std::string& ToString() const;
  bool empty() const;
};

bool operator==(const StringWrapper& a, const StringWrapper& b);
bool operator!=(const StringWrapper& a, const StringWrapper& b);
bool operator==(const StringWrapper& a, const std::string& b);
bool operator!=(const StringWrapper& a, const std::string& b);
bool operator==(const std::string& b, const StringWrapper& a);
bool operator!=(const std::string& b, const StringWrapper& a);

// All data types use StringWrapper instead of strings. It allows to eliminate
// expensive string operations when objects are copied. But it forces all
// strings used to construct the objects to outlive all objects. Use StringPool
// to keep all strings alive.
class StringPool {
 public:
  StringWrapper Add(const std::string& s);
  size_t Size() const;

 protected:
  std::mutex mutex_;
  std::forward_list<std::string> strings_;
  std::unordered_map<std::string_view, std::string*> index_;
};

using SnakeId = StringWrapper;

enum class Move {
  Unknown = 0,  // No move returned from snake.
  Up,
  Down,
  Left,
  Right,
};

struct EliminatedCause {
  enum Cause {
    NotEliminated = 0,
    Collision,
    SelfCollision,
    OutOfHealth,
    HeadToHeadCollision,
    OutOfBounds,
    BySquad,
  };

  Cause cause;
  SnakeId by_id;
};

struct Point {
  Coordinate x;
  Coordinate y;

  bool operator==(const Point& other) const {
    return this->x == other.x && this->y == other.y;
  }
  bool operator!=(const Point& other) const { return !operator==(other); }

  Point Up() const { return Point{x, static_cast<Coordinate>(y + 1)}; }
  Point Down() const { return Point{x, static_cast<Coordinate>(y - 1)}; }
  Point Left() const { return Point{static_cast<Coordinate>(x - 1), y}; }
  Point Right() const { return Point{static_cast<Coordinate>(x + 1), y}; }
  Point Moved(Move move) const;
};

// Max board area is kBoardSizeMax^2, but snakes may have an extra element at
// their tail and may go out of bounds, so extra buffer of 2 elements. This
// vector never allocates memory on heap, thus improving performance. Though it
// uses more memory than regular std::vector in most cases.
static constexpr int kMaxSnakeBodyLen = kBoardSizeMax * kBoardSizeMax + 2;

using PointsVector = ::theapx::trivial_loop_array<Point, kMaxSnakeBodyLen>;

struct PointHash {
  size_t operator()(const Point& point) const {
    size_t x_hash = std::hash<int>()(point.x);
    size_t y_hash = std::hash<int>()(point.y) << 1;
    return x_hash ^ y_hash;
  }
};

struct SnakeBody : public theapx::trivial_loop_array<Point, kMaxSnakeBodyLen> {
  Point& Head() { return front(); }
  const Point& Head() const { return front(); }
  int Length() const { return size(); }
  Point& Piece(int n) { return at(n); }
  const Point& Piece(int n) const { return at(n); }

  void MoveTo(Move move);
  void IncreaseLength(int delta = 1);

  template <class T>
  static SnakeBody Create(const T& data) {
    SnakeBody result{};
    for (const Point& p : data) {
      result.push_back(p);
    }
    return result;
  }

  static SnakeBody Create(const std::initializer_list<Point>& data) {
    return Create<std::initializer_list<Point>>(data);
  }
};

bool operator==(const SnakeBody& a, const SnakeBody& b);
inline bool operator!=(const SnakeBody& a, const SnakeBody& b) {
  return !(a == b);
}

struct Snake {
  // Main values used by the engine.
  SnakeId id;
  SnakeBody body;
  int health;
  EliminatedCause eliminated_cause;

  // Additional values not necessarily used by ruleset, but used in API.
  StringWrapper name;
  StringWrapper latency;
  StringWrapper shout;
  StringWrapper squad;

  bool IsEliminated() const {
    return eliminated_cause.cause != EliminatedCause::NotEliminated;
  }

  bool IsOutOfHealth() const { return health <= 0; }

  Point& Head();
  const Point& Head() const;
  size_t Length() const { return body.size(); }
};

using SnakesVector = ::theapx::trivial_loop_array<Snake, kSnakesCountMax>;

struct BoardState {
  Coordinate width;
  Coordinate height;
  PointsVector food;
  SnakesVector snakes;
  PointsVector hazards;
};

struct RulesetInfo {
  StringWrapper name;
  StringWrapper version;
};

struct GameInfo {
  StringWrapper id;
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

std::ostream& operator<<(std::ostream& s, const StringWrapper& string);
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

namespace std {
template <>
struct hash<battlesnake::rules::StringWrapper> {
  std::size_t operator()(
      battlesnake::rules::StringWrapper const& s) const noexcept {
    if (s.value == nullptr) return 0;
    return std::hash<std::string>{}(*s.value);
  }
};
}  // namespace std
