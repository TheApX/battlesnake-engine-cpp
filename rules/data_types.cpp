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

Move DetectMove(const Point& from, const Point& to) {
  if (to == from.Up()) return Move::Up;
  if (to == from.Down()) return Move::Down;
  if (to == from.Left()) return Move::Left;
  if (to == from.Right()) return Move::Right;
  return Move::Unknown;
}

SnakeBody::Piece SnakeBody::Piece::Next() const {
  if (!Valid()) {
    return *this;
  }
  if (index_ == body_->Length() - 1) {
    return Piece(body_, index_ + 1, pos_);
  }
  if (body_->NextRepeated(index_)) {
    return Piece(body_, index_ + 1, pos_);
  }
  return Piece(body_, index_ + 1, pos_.Moved(body_->NextMove(index_)));
}

bool SnakeBody::Piece::operator==(const SnakeBody::Piece& other) const {
  if (!this->Valid() && !other.Valid()) return true;
  if (this->Valid() != other.Valid()) return false;

  if (this->body_ != other.body_) return false;
  if (this->index_ != other.index_) return false;

  return true;
}

void SnakeBody::MoveTo(Move move) {
  if (move == Move::Unknown) {
    if (size() >= 2) {
      move = DetectMove(Head().Next().Pos(), Head().Pos());
    }
  }
  if (move == Move::Unknown) {
    move = Move::Up;
  }

  head = head.Moved(move);
  if (moves_offset == 0) {
    moves.push_front(0);
    moves_offset = kMovesPerBlock;
  }

  BlockType& block = moves.front();
  moves_offset--;

  BlockType move_data = static_cast<BlockType>(Opposite(move))
                        << (moves_offset * 2);
  block = block | move_data;

  moves_length = std::min(moves_length + 1, total_length - 1);
  int moves_with_offset = moves_offset + moves_length;
  int blocks_needed = moves_with_offset / kMovesPerBlock +
                      (moves_with_offset % kMovesPerBlock == 0 ? 0 : 1);
  if (moves.size() > blocks_needed) {
    moves.resize(blocks_needed);
  }
}

void SnakeBody::IncreaseLength(int delta) { total_length += delta; }

Move SnakeBody::NextMove(short index) const {
  short index_moves_offset = index + moves_offset;

  short index_block = index_moves_offset / kMovesPerBlock;
  unsigned char block_data = moves.at(index_block);

  unsigned short index_block_offset = index_moves_offset % kMovesPerBlock;
  return static_cast<Move>((block_data >> (index_block_offset * 2)) & 0x03u);
}

bool operator==(const SnakeBody& a, const SnakeBody& b) {
  if (a.head != b.head) {
    return false;
  }
  if (a.total_length != b.total_length) {
    return false;
  }
  if (a.moves_length != b.moves_length) {
    return false;
  }

  for (int i = 0; i < a.moves_length; ++i) {
    if (a.NextMove(i) != b.NextMove(i)) {
      return false;
    }
  }

  return true;
}

bool BoardBits::Get(int index) const {
  int block_index = index / kBlockSizeBits;
  int block_offset = index % kBlockSizeBits;
  BlockType bit_block_value = static_cast<BlockType>(1)
                              << static_cast<BlockType>(block_offset);
  return (data[block_index] & bit_block_value) != 0;
}

void BoardBits::Set(int index, bool value) {
  int block_index = index / kBlockSizeBits;
  int block_offset = index % kBlockSizeBits;
  BlockType bit_block_value = static_cast<BlockType>(1)
                              << static_cast<BlockType>(block_offset);
  BlockType old_value = data[block_index];
  if (value) {
    data[block_index] |= bit_block_value;
  } else {
    data[block_index] &= ~bit_block_value;
  }
}

bool operator==(const BoardBits& a, const BoardBits& b) {
  return std::memcmp(&a, &b, sizeof(a)) == 0;
}

template <class BitsType>
BoardBitsViewBase<BitsType>::BitsIterator::BitsIterator(
    const BoardBitsViewBase<BitsType>* owner, int index) {
  owner_ = owner;
  block_index_ = index / BoardBits::kBlockSizeBits;
  block_offset_ = index % BoardBits::kBlockSizeBits;

  AdvanceToNextPoint();
}

template <class BitsType>
void BoardBitsViewBase<BitsType>::BitsIterator::AdvanceToNextPoint() {
  // Find a `1` bit starting from current position.
  while (IsValid()) {
    BoardBits::BlockType block = owner_->bits_->data[block_index_];
    BoardBits::BlockType bit_block_value =
        static_cast<BoardBits::BlockType>(1)
        << static_cast<BoardBits::BlockType>(block_offset_);
    bool bit = (block & bit_block_value) != 0;
    if (bit) break;

    if ((block >> block_offset_) == 0) {
      // No `1` bits left in the current block. Move to the next one.
      block_offset_ = 0;
      block_index_++;
      continue;
    }

    while (IsValid() && block_offset_ < BoardBits::kBlockSizeBits) {
      block_offset_++;
      BoardBits::BlockType bit_block_value =
          static_cast<BoardBits::BlockType>(1)
          << static_cast<BoardBits::BlockType>(block_offset_);
      bit = (block & bit_block_value) != 0;
      if (bit) break;
    }
  }
}

template <class BitsType>
bool BoardBitsViewBase<BitsType>::BitsIterator::IsValid() const {
  return block_index_ * BoardBits::kBlockSizeBits + block_offset_ <
         owner_->width_ * owner_->height_;
}

template <class BitsType>
BoardBitsViewBase<BitsType>::BitsIterator
BoardBitsViewBase<BitsType>::BitsIterator::operator++() {
  block_offset_++;
  if (block_offset_ == BoardBits::kBlockSizeBits) {
    block_offset_ = 0;
    block_index_++;
  }

  AdvanceToNextPoint();
  return *this;
}

template <class BitsType>
BoardBitsViewBase<BitsType>::BitsIterator
BoardBitsViewBase<BitsType>::BitsIterator::operator++(int) {
  BitsIterator result = *this;

  block_offset_++;
  if (block_offset_ == BoardBits::kBlockSizeBits) {
    block_offset_ = 0;
    block_index_++;
  }

  AdvanceToNextPoint();

  return result;
}

template <class BitsType>
Point BoardBitsViewBase<BitsType>::BitsIterator::operator*() const {
  int index = block_index_ * BoardBits::kBlockSizeBits + block_offset_;
  return Point{
      .x = static_cast<Coordinate>(index % owner_->width_),
      .y = static_cast<Coordinate>(index / owner_->width_),
  };
}

template <class BitsType>
bool BoardBitsViewBase<BitsType>::BitsIterator::operator==(
    const BoardBitsViewBase::BitsIterator& other) const {
  if (!IsValid() && !other.IsValid()) {
    return true;
  }

  if (!IsValid() || !other.IsValid()) {
    return false;
  }

  if (owner_ != other.owner_) return false;
  if (block_index_ != other.block_index_) return false;
  if (block_offset_ != other.block_offset_) return false;

  return true;
}

template class BoardBitsViewBase<BoardBits>;
template class BoardBitsViewBase<const BoardBits>;

bool operator==(const HazardInfo& a, const HazardInfo& b) {
  if (a.depth_left != b.depth_left) return false;
  if (a.depth_right != b.depth_right) return false;
  if (a.depth_top != b.depth_top) return false;
  if (a.depth_bottom != b.depth_bottom) return false;

  return true;
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
