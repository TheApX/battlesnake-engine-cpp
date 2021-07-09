#include "battlesnake/rules/data_types.h"

#include "battlesnake/rules/errors.h"

namespace battlesnake {
namespace rules {

namespace {

template <class T>
std::ostream& PrintContainer(std::ostream& s, const T& v) {
  s << "[";
  bool first = true;
  for (const auto& t : v) {
    if (!first) {
      s << ",";
    }
    s << t;
    first = false;
  }
  s << "]";
  return s;
}

}  // namespace

Point& Snake::Head() {
  if (body.empty()) {
    throw ErrorZeroLengthSnake(id);
  }
  return body.front();
}

const Point& Snake::Head() const {
  if (body.empty()) {
    throw ErrorZeroLengthSnake(id);
  }
  return body.front();
}

std::ostream& operator<<(std::ostream& s, Move move) {
  switch (move) {
    case Move::Unknown:
      s << "Unknown";
      break;
    case Move::Up:
      s << "Up";
      break;
    case Move::Down:
      s << "Down";
      break;
    case Move::Left:
      s << "Left";
      break;
    case Move::Right:
      s << "Right";
      break;

    default:
      s << "Unknown";
      break;
  }
  return s;
}

std::ostream& operator<<(std::ostream& s, EliminatedCause::Cause cause) {
  switch (cause) {
    case EliminatedCause::NotEliminated:
      s << "NotEliminated";
      break;
    case EliminatedCause::Collision:
      s << "Collision";
      break;
    case EliminatedCause::SelfCollision:
      s << "SelfCollision";
      break;
    case EliminatedCause::OutOfHealth:
      s << "OutOfHealth";
      break;
    case EliminatedCause::HeadToHeadCollision:
      s << "HeadToHeadCollision";
      break;
    case EliminatedCause::OutOfBounds:
      s << "OutOfBounds";
      break;

    default:
      s << "Unknown";
      break;
  }
  return s;
}

std::ostream& operator<<(std::ostream& s, EliminatedCause cause) {
  s << cause.cause;
  switch (cause.cause) {
    case EliminatedCause::Collision:
    case EliminatedCause::HeadToHeadCollision:
      s << " by '" << cause.by_id << "'";
      break;
    default:
      break;
  }
  return s;
}

std::ostream& operator<<(std::ostream& s, const Point& point) {
  s << "(" << point.x << "," << point.y << ")";
  return s;
}

std::ostream& operator<<(std::ostream& s, const Snake& snake) {
  s << "{";
  s << "id: '" << snake.id << "'";
  s << " body: ";
  PrintContainer(s, snake.body);
  s << " health: " << snake.health;
  s << " eliminated: " << snake.eliminated_cause;
  s << "}";

  return s;
}

}  // namespace rules
}  // namespace battlesnake
