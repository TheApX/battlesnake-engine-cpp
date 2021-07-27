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

std::string_view StringPool::Add(const std::string& s) {
  std::lock_guard guard(mutex_);

  auto it = index_.find(s);
  if (it != index_.end()) {
    return *it->second;
  }

  strings_.push_front(s);
  std::string* new_string = &strings_.front();
  index_[*new_string] = new_string;
  return std::string_view(*new_string);
}

size_t StringPool::Size() const { return index_.size(); }

Point Point::Moved(Move move) const {
  switch (move) {
    case Move::Up:
      return Up();
    case Move::Down:
      return Down();
    case Move::Left:
      return Left();
    case Move::Right:
      return Right();

    default:
      return *this;
  }
}

Point& Snake::Head() {
  if (body.empty()) {
    throw ErrorZeroLengthSnake(std::string(id));
  }
  return body.front();
}

const Point& Snake::Head() const {
  if (body.empty()) {
    throw ErrorZeroLengthSnake(std::string(id));
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
  s << " name: '" << snake.name << '\'';
  s << " latency: " << snake.latency;
  s << " shout: '" << snake.shout << '\'';
  s << " squad: '" << snake.squad << '\'';
  s << "}";

  return s;
}

}  // namespace rules
}  // namespace battlesnake
