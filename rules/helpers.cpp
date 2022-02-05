#include "battlesnake/rules/helpers.h"

#include <algorithm>
#include <string>
#include <unordered_map>
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

std::string RenderGame(
    const GameState& state,
    const std::unordered_map<SnakeId, char>& snake_head_syms) {
  auto ind = [&](int x, int y) -> int { return y * state.board.width + x; };

  std::vector<std::string> board(state.board.width * state.board.height,
                                 kSpaceSymbol);

  for (const Point& p : state.board.Hazard()) {
    board[ind(p.x, p.y)] = kHazardSymbol;
  }

  for (const Point& pos : state.board.Food()) {
    board[ind(pos.x, pos.y)] = kFoodSymbol;
  }

  for (const Snake& snake : state.board.snakes) {
    if (snake.Length() == 0) {
      continue;
    }
    if (snake.IsEliminated()) {
      continue;
    }

    SnakeBody::Piece last_pos = snake.body.Head();
    SnakeBody::Piece pos = last_pos.Next();
    while (pos.Valid()) {
      SnakeBody::Piece prev = last_pos;
      SnakeBody::Piece next = pos.Next();
      if (!next.Valid()) {
        break;
      }
      if (next.Pos() == pos.Pos()) {
        last_pos = pos;
        pos = next;
        continue;
      }

      Direction dp = GetDirection(pos.Pos(), prev.Pos());
      Direction dn = GetDirection(pos.Pos(), next.Pos());
      board[ind(pos.Pos().x, pos.Pos().y)] = kBodySymbols[dn][dp];

      last_pos = pos;
      pos = next;
    }

    board[ind(pos.Pos().x, pos.Pos().y)] =
        kTailSymbols[GetDirection(last_pos.Pos(), pos.Pos())];

    Direction head_dir = Unknown;
    if (snake.Length() > 1) {
      Point next = snake.body.Head().Next().Pos();
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
                   std::to_string(snake.Length()) + "  " +
                   snake.name.ToString() + "  " + snake.latency.ToString() +
                   "ms  " + snake.squad.ToString());
  }

  std::string result;
  for (const std::string& l : lines) {
    result += l + "\n";
  }
  return result;
}

}  // namespace rules
}  // namespace battlesnake
