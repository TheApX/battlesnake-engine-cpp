#include <exception>
#include <string>
#include <vector>
#include <map>

namespace battlesnake {
namespace engine {

// Standard board sizes.
static constexpr int kBoardSizeSmall  = 7;
static constexpr int kBoardSizeMedium = 11;
static constexpr int kBoardSizeLarge  = 19;

using SnakeId = std::string;

enum class Move {
  Up,
  Down,
  Left,
  Right,
};

enum class EliminatedCause {
  NotEliminated,
  ByCollision,
  BySelfCollision,
  ByOutOfHealth,
  ByHeadToHeadCollision,
  ByOutOfBounds,
};

struct Point {
  Point() = default;
  Point(const Point&) = default;
  Point(Point&&) = default;

  Point(int x_, int y_) : x(x_), y(y_) {}

  bool operator==(const Point& other) const {
    return this->x == other.x && this->y == other.y;
  }

  int x = 0;
  int y = 0;
};

struct Snake {
  SnakeId id;
	std::vector<Point> body;
  int health = 0;
	EliminatedCause eliminated_cause = EliminatedCause::NotEliminated;
	SnakeId eliminated_by_id;
};

struct BoardState {
  int height = 0;
  int width = 0;
  std::vector<Point> food;
  std::vector<Snake> snakes;
};

class Ruleset {
 public:
  virtual ~Ruleset() = default;

  virtual BoardState CreateInitialBoardState(int width, int height, std::vector<SnakeId> snakeIDs) = 0;
  virtual BoardState CreateNextBoardState(const BoardState& prev_state, std::map<SnakeId, Move> moves) = 0;
  virtual bool IsGameOver(const BoardState& state) = 0;
};

}  // namespace engine
}  // namespace battlesnake
