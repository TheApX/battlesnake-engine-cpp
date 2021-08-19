#include "battlesnake/rules/data_types.h"

#include <cstring>

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

const std::string& StringWrapper::ToString() const {
  static const std::string empty = "";
  if (value == nullptr) {
    return empty;
  }
  return *value;
}

bool StringWrapper::empty() const {
  if (value == nullptr) return true;
  return value->empty();
}

bool operator==(const StringWrapper& a, const StringWrapper& b) {
  if (a.value == b.value) return true;
  if (a.value == nullptr) return b.empty();
  if (b.value == nullptr) return a.empty();
  return *a.value == *b.value;
}

bool operator!=(const StringWrapper& a, const StringWrapper& b) {
  return !(a == b);
}

bool operator==(const StringWrapper& a, const std::string& b) {
  if (a.value == nullptr) return b.empty();
  return *a.value == b;
}

bool operator!=(const StringWrapper& a, const std::string& b) {
  return !(a == b);
}

bool operator==(const std::string& b, const StringWrapper& a) { return a == b; }
bool operator!=(const std::string& b, const StringWrapper& a) { return a != b; }

StringWrapper StringPool::Add(const std::string& s) {
  std::lock_guard guard(mutex_);

  auto it = index_.find(s);
  if (it != index_.end()) {
    return StringWrapper{.value = it->second};
  }

  strings_.push_front(s);
  std::string* new_string = &strings_.front();
  index_[*new_string] = new_string;
  return StringWrapper{.value = new_string};
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

void SnakeBody::MoveTo(Move move) {
  push_front(Head().Moved(move));
  resize(size() - 1);
}

void SnakeBody::IncreaseLength(int delta) {
  for (int i = 0; i < delta; ++i) {
    push_back(back());
  }
}

bool operator==(const SnakeBody& a, const SnakeBody& b) {
  if (a.size() != b.size()) {
    return false;
  }

  for (int i = 0; i < a.size(); ++i) {
    if (a.at(i) != b.at(i)) {
      return false;
    }
  }

  return true;
}

Point& Snake::Head() {
  if (body.empty()) {
    throw ErrorZeroLengthSnake(id.ToString());
  }
  return body.front();
}

const Point& Snake::Head() const {
  if (body.empty()) {
    throw ErrorZeroLengthSnake(id.ToString());
  }
  return body.front();
}

std::ostream& operator<<(std::ostream& s, const StringWrapper& string) {
  return s << string.ToString();
}

std::ostream& operator<<(std::ostream& s, Move move) {
  switch (move) {
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
  s << "(" << static_cast<int>(point.x) << "," << static_cast<int>(point.y)
    << ")";
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
