#include "battlesnake/rules/helpers.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace battlesnake {
namespace rules {

namespace {

enum Direction {
  Left = 0,
  Right = 1,
  Up = 2,
  Down = 3,
  Unknown = 4,
};

constexpr char const* kHeadSymbols[5] = {
    " X", "═X", " X", " X", " X",
};

constexpr char const* kTailSymbols[5] = {
    " ╞", "═╡", " ╥", " ╨", " ?",
};

constexpr char const* kBodySymbols[5][5] = {
    {
        // Left
        " ╬",
        "══",
        "═╝",
        "═╗",
        " ╬",
    },
    {
        // Right
        "══",
        " ╬",
        " ╚",
        " ╔",
        " ╬",
    },
    {
        // Up
        "═╝",
        " ╚",
        " ╬",
        " ║",
        " ╬",
    },
    {
        // Down
        "═╗",
        " ╔",
        " ║",
        " ╬",
        " ╬",
    },
    {
        // Unknown
        " ╬",
        " ╬",
        " ╬",
        " ╬",
        " ╬",
    },
};

constexpr char kSpaceSymbol[] = "  ";
constexpr char kFoodSymbol[] = " o";
constexpr char kHazardSymbol[] = "▒▒";

Direction GetDirection(const Point& f, const Point& t) {
  if (f.x - 1 == t.x && f.y == t.y) return Left;
  if (f.x + 1 == t.x && f.y == t.y) return Right;
  if (f.x == t.x && f.y - 1 == t.y) return Down;
  if (f.x == t.x && f.y + 1 == t.y) return Up;
  return Unknown;
}

std::string NumToStr(int n, int len) {
  std::string r = std::to_string(n);
  if (r.size() < len) {
    return std::string(len - r.size(), ' ') + r;
  }
  return r;
}

void AppendLine(std::vector<std::string>& lines, int len, int n,
                const std::string& s) {
  if (lines.size() <= n) {
    std::string extra(len, ' ');
    lines.reserve(n + 1);
    for (int i = lines.size(); i <= n; ++i) {
      lines.push_back(extra);
    }
  }

  lines[n] += "  " + s;
}

}  // namespace

std::string RenderGame(const GameState& state,
                       const std::map<SnakeId, char>& snake_head_syms) {
  auto ind = [&](int x, int y) -> int { return y * state.board.width + x; };

  std::vector<std::string> board(state.board.width * state.board.height,
                                 kSpaceSymbol);

  for (const Point& pos : state.board.hazards) {
    board[ind(pos.x, pos.y)] = kHazardSymbol;
  }
  for (const Point& pos : state.board.food) {
    board[ind(pos.x, pos.y)] = kFoodSymbol;
  }

  for (const Snake& snake : state.board.snakes) {
    if (snake.Length() == 0) {
      continue;
    }

    Point last_pos = snake.Head();
    for (int i = 1; i < snake.Length() - 1; ++i) {
      Point pos = snake.body[i];
      Point prev = snake.body[i - 1];
      Point next = snake.body[i + 1];
      if (next == pos) {
        continue;
      }
      last_pos = pos;
      Direction dp = GetDirection(pos, prev);
      Direction dn = GetDirection(pos, next);
      board[ind(pos.x, pos.y)] = kBodySymbols[dn][dp];
    }

    Point tail = snake.body[snake.body.size() - 1];
    board[ind(tail.x, tail.y)] = kTailSymbols[GetDirection(last_pos, tail)];

    Direction head_dir = Unknown;
    if (snake.Length() > 1) {
      Point next = snake.body[1];
      head_dir = GetDirection(next, snake.Head());
    }
    std::string head_board = kHeadSymbols[head_dir];
    auto head_char = snake_head_syms.find(snake.id);
    if (head_char != snake_head_syms.end()) {
      std::replace(head_board.begin(), head_board.end(), 'X',
                   head_char->second);
    }

    board[ind(snake.Head().x, snake.Head().y)] = head_board;
  }

  std::vector<std::string> lines;

  {
    std::string l = "   ";
    for (int x = 0; x < state.board.width; ++x) {
      l += " " + std::to_string(x % 10);
    }
    lines.push_back(l + "  ");
  }

  for (int y = state.board.height - 1; y >= 0; --y) {
    std::string l = NumToStr(y, 2) + " ";
    for (int x = 0; x < state.board.width; ++x) {
      l += board[ind(x, y)];
    }
    lines.push_back(l + " <");
  }

  int board_len = 5 + state.board.width * 2;
  lines.push_back(std::string(board_len, '^'));

  int n = 0;
  AppendLine(lines, board_len, n++, "Turn: " + std::to_string(state.turn));

  for (const Snake& snake : state.board.snakes) {
    auto head_char_it = snake_head_syms.find(snake.id);
    std::string head_char = head_char_it == snake_head_syms.end()
                                ? "X"
                                : std::string(1, head_char_it->second);
    AppendLine(lines, board_len, n++,
               head_char + ":  " + std::to_string(snake.health) + "  " +
                   std::to_string(snake.Length()) + "  " + snake.name + "  " +
                   snake.latency + "ms");
  }

  std::string result;
  for (const std::string& l : lines) {
    result += l + "\n";
  }
  return result;
}

}  // namespace rules
}  // namespace battlesnake
