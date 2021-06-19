#include <exception>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace battlesnake {
namespace engine {

// Standard board sizes.
static constexpr int kBoardSizeSmall = 7;
static constexpr int kBoardSizeMedium = 11;
static constexpr int kBoardSizeLarge = 19;

using SnakeId = std::string;

enum class Move {
  Up,
  Down,
  Left,
  Right,
};

enum class EliminatedCause {
  NotEliminated,
  Collision,
  SelfCollision,
  OutOfHealth,
  HeadToHeadCollision,
  OutOfBounds,
};

struct Point {
  int x = 0;
  int y = 0;

  Point() = default;
  Point(const Point&) = default;
  Point(Point&&) = default;

  Point(int x_, int y_) : x(x_), y(y_) {}

  Point& operator=(const Point& other) = default;
  Point& operator=(Point&& other) = default;

  bool operator==(const Point& other) const {
    return this->x == other.x && this->y == other.y;
  }

  Point Up() const { return Point(x, y + 1); }
  Point Down() const { return Point(x, y - 1); }
  Point Left() const { return Point(x - 1, y); }
  Point Right() const { return Point(x + 1, y); }
};

struct PointHash {
  size_t operator()(const Point& point) const {
    size_t x_hash = std::hash<int>()(point.x);
    size_t y_hash = std::hash<int>()(point.y) << 1;
    return x_hash ^ y_hash;
  }
};

struct Snake {
  SnakeId id;
  std::vector<Point> body;
  int health = 0;
  EliminatedCause eliminated_cause = EliminatedCause::NotEliminated;
  SnakeId eliminated_by_id;

  bool IsEliminated() const {
    return eliminated_cause != EliminatedCause::NotEliminated;
  }
};

struct BoardState {
  int width = 0;
  int height = 0;
  std::vector<Point> food;
  std::vector<Snake> snakes;
};

std::ostream& operator<<(std::ostream& s, Move move);
std::ostream& operator<<(std::ostream& s, EliminatedCause cause);
std::ostream& operator<<(std::ostream& s, const Point& point);
std::ostream& operator<<(std::ostream& s, const Snake& snake);
// TODO: implement when needed.
// std::ostream& operator<<(std::ostream& s, const BoardState& state);

class Ruleset {
 public:
  virtual ~Ruleset() = default;

  virtual BoardState CreateInitialBoardState(int width, int height,
                                             std::vector<SnakeId> snakeIDs) = 0;
  virtual BoardState CreateNextBoardState(const BoardState& prev_state,
                                          std::map<SnakeId, Move> moves) = 0;
  virtual bool IsGameOver(const BoardState& state) = 0;
};

}  // namespace engine
}  // namespace battlesnake
