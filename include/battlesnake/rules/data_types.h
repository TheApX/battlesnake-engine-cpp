#pragma once

#include <battlesnake/rules/errors.h>

#include <cstdint>
#include <cstring>
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

// Move direction. Values are chosen to simplify calculation of the opposite
// direction.
enum class Move {
  Up = 0,
  Down = 3,
  Left = 1,
  Right = 2,

  Unknown = 4,
};

inline Move Opposite(Move m) {
  return static_cast<Move>(3 - static_cast<int>(m));
}

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

Move DetectMove(const Point& from, const Point& to);

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

struct SnakeBody {
 private:
  struct fake_allocator {
    typedef Point value_type;
    typedef Point* pointer;
    typedef const Point* const_pointer;
  };

 public:
  class Piece {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Point;
    using reference = Point&;
    using const_reference = const Point&;
    using pointer = Point*;
    using const_pointer = const Point*;
    using allocator_type = fake_allocator;

    Piece(const SnakeBody* body, short index, Point pos)
        : body_(body), index_(index), pos_(pos) {}

    bool Valid() const { return index_ < body_->Length(); }
    const Point& Pos() const { return pos_; }
    Piece Next() const;

    Piece operator++() {
      *this = Next();
      return *this;
    }
    Piece operator++(int) {
      Piece result = *this;
      *this = Next();
      return result;
    }

    const Point& operator*() { return Pos(); }
    const Point* operator->() { return &Pos(); }

    bool operator==(const Piece& other) const;
    bool operator!=(const Piece& other) const { return !operator==(other); }

   private:
    const SnakeBody* body_;
    short index_;
    Point pos_;
  };

  using value_type = Point;
  using reference = Point&;
  using const_reference = const Point&;
  using pointer = Point*;
  using const_pointer = const Point*;
  using iterator = Piece;
  using const_iterator = Piece;
  using allocator_type = fake_allocator;

  const Point& HeadPos() const { return head; }
  const Point& TailPos() const { return tail; }
  Piece Head() const { return Piece(this, 0, head); }
  int Length() const { return total_length; }

  void MoveTo(Move move);
  void IncreaseLength(int delta = 1);

  bool NextRepeated(short index) const { return index >= moves_length; }
  Move NextMove(short index) const;

  bool empty() const { return total_length == 0; }
  int size() const { return total_length; }

  Piece begin() const { return Head(); }
  Piece end() const { return Piece(this, total_length, head); }

  // Example: {1,1}, {1,2}, {2,2}, {2,3}, {2,3}, {2,3}
  // Assuming sizeof(BlockType) == 1
  // head = {1,1}
  // total_length = 6
  // moves_length = 3
  // moves = [Left, Up, Left] packed 2 bits per move
  // moves_offset = 0, 1, 2 or 3, depending on the offset of the first move in
  //                the first block in `moves`

  using BlockType = unsigned char;
  static constexpr int kMovesPerBlock = sizeof(BlockType) * 4;
  static constexpr short kBodyDataLength =
      kMaxSnakeBodyLen / kMovesPerBlock +
      (kMaxSnakeBodyLen % kMovesPerBlock == 0 ? 0 : 1);
  using BodyMovesVector =
      ::theapx::trivial_loop_array<BlockType, kBodyDataLength>;

  Point head;
  Point tail;
  short total_length;
  short moves_length;
  BodyMovesVector moves;
  signed char moves_offset;

  template <class T>
  static SnakeBody Create(const T& data) {
    SnakeBody result{
        .total_length = static_cast<short>(data.size()),
        .moves_length = 0,
        .moves = {},
        .moves_offset = 0,
    };

    if (data.size() != 0) {
      result.head = *data.begin();
    }

    Point prev = result.head;
    bool first = true;
    BlockType current_block = 0;
    int block_offset = 0;
    for (Point p : data) {
      if (first) {
        first = false;
        continue;
      }
      Move move = DetectMove(prev, p);
      if (move == Move::Unknown) {
        // if (prev != p) {
        //   throw RulesetException("Invalid body data");
        // }
        break;
      }
      result.moves_length++;
      BlockType current_move = static_cast<BlockType>(move);
      current_block = current_block | (current_move << (block_offset * 2));
      block_offset++;
      if (block_offset == kMovesPerBlock) {
        block_offset = 0;
        result.moves.push_back(current_block);
        current_block = 0;
      }

      prev = p;
    }
    result.tail = prev;

    if (block_offset != 0) {
      result.moves.push_back(current_block);
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

  Point& Head() { return body.head; }
  const Point& Head() const { return body.head; }
  size_t Length() const { return body.Length(); }
};

struct BoardBits {
 public:
  using BlockType = uint64_t;
  static constexpr int kBitsSizeNeeded = kBoardSizeMax * kBoardSizeMax;
  static constexpr int kBlockSizeBytes = sizeof(BlockType);
  static constexpr int kBlockSizeBits = kBlockSizeBytes * 8;
  static constexpr int kBlocksCount =
      kBitsSizeNeeded / kBlockSizeBits +
      (kMaxSnakeBodyLen % kBlockSizeBits == 0 ? 0 : 1);
  static constexpr int kMaxBitsSize = kBlocksCount * kBlockSizeBits;

  BlockType data[kBlocksCount];

  bool Get(int index) const;
  void Set(int index, bool value);

  void clear() { std::memset(this, 0, sizeof(*this)); }
};

bool operator==(const BoardBits& a, const BoardBits& b);
inline bool operator!=(const BoardBits& a, const BoardBits& b) {
  return !(a == b);
}

template <class BitsType>
class BoardBitsViewBase {
 private:
  struct fake_allocator {
    typedef Point value_type;
    typedef Point* pointer;
    typedef const Point* const_pointer;
  };

 public:
  BoardBitsViewBase(BitsType* bits, int width, int height)
      : bits_(bits), width_(width), height_(height) {}

  bool Get(Point p) const { return bits_->Get(p.y * width_ + p.x); }

  int Count() const {
    int result = 0;
    for (const auto& p : *this) result++;
    return result;
  }

  class BitsIterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Point;
    using allocator_type = fake_allocator;

    BitsIterator(const BoardBitsViewBase<BitsType>* owner, int index);
    void AdvanceToNextPoint();

    bool IsValid() const;

    BitsIterator operator++();
    BitsIterator operator++(int);

    Point operator*() const;

    bool operator==(const BitsIterator& other) const;
    bool operator!=(const BitsIterator& other) const {
      return !operator==(other);
    }

   private:
    const BoardBitsViewBase<BitsType>* owner_;
    int block_index_;
    int block_offset_;
  };

  using value_type = Point;
  using iterator = BitsIterator;
  using const_iterator = BitsIterator;
  using allocator_type = fake_allocator;

  BitsIterator begin() const { return BitsIterator(this, 0); }
  BitsIterator end() const { return BitsIterator(this, width_ * height_); }

 protected:
  BitsType* bits_;
  int width_;
  int height_;
};

class BoardBitsView : public BoardBitsViewBase<BoardBits> {
 public:
  BoardBitsView(BoardBits* bits, int width, int height)
      : BoardBitsViewBase<BoardBits>(bits, width, height) {}

  void Set(Point p, bool value) { bits_->Set(p.y * width_ + p.x, value); }

  template <class T>
  void Fill(const T& data) {
    std::memset(bits_, 0, sizeof(*bits_));
    for (Point p : data) {
      Set(p, true);
    }
  }

  void Fill(const std::initializer_list<Point>& data) {
    Fill<std::initializer_list<Point>>(data);
  }
};

class BoardBitsViewConst : public BoardBitsViewBase<const BoardBits> {
 public:
  BoardBitsViewConst(const BoardBits* bits, int width, int height)
      : BoardBitsViewBase<const BoardBits>(bits, width, height) {}
};

template <class T>
inline BoardBits CreateBoardBits(const T& data, Coordinate width,
                                 Coordinate height) {
  BoardBits result{};
  BoardBitsView view(&result, width, height);
  view.Fill(data);
  return result;
}

inline BoardBits CreateBoardBits(const std::initializer_list<Point>& data,
                                 Coordinate width, Coordinate height) {
  return CreateBoardBits<std::initializer_list<Point>>(data, width, height);
}

using SnakesVector = ::theapx::trivial_loop_array<Snake, kSnakesCountMax>;
struct BoardState {
  Coordinate width;
  Coordinate height;
  BoardBits food;
  SnakesVector snakes;
  BoardBits hazard;

  BoardBitsView Food() { return BoardBitsView(&this->food, width, height); }
  BoardBitsViewConst Food() const {
    return BoardBitsViewConst(&this->food, width, height);
  }

  BoardBitsView Hazard() { return BoardBitsView(&this->hazard, width, height); }
  BoardBitsViewConst Hazard() const {
    return BoardBitsViewConst(&this->hazard, width, height);
  }

  bool InHazard(const Point& p) const { return Hazard().Get(p); }
};

struct RulesetSettings {
  int food_spawn_chance;
  int minimum_food;
  int hazard_damage_per_turn;
  int royale_shrink_every_n_turns;
  bool squad_allow_body_collisions;
  bool squad_shared_elimination;
  bool squad_shared_health;
  bool squad_shared_length;
};

struct RulesetInfo {
  StringWrapper name;
  StringWrapper version;
  RulesetSettings settings;
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
