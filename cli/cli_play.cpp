#include <curl/curl.h>
#include <uuid.h>

#include <chrono>
#include <memory>
#include <unordered_map>

#include "battlesnake/json/converter.h"
#include "battlesnake/rules/helpers.h"
#include "battlesnake/rules/ruleset.h"
#include "battlesnake/rules/standard_ruleset.h"
#include "cli_options.h"
#include "http_client_battlesnake.h"

namespace battlesnake {
namespace cli {

namespace {

using namespace ::battlesnake::rules;
using namespace ::battlesnake::interface;
using namespace ::battlesnake::json;

class CurlInit {
 public:
  CurlInit() { curl_global_init(CURL_GLOBAL_ALL); }
  ~CurlInit() { curl_global_cleanup(); }
};

std::string GenerateId() {
  std::random_device rd;
  auto seed_data = std::array<int, std::mt19937::state_size>{};
  std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
  std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
  std::mt19937 generator(seq);
  uuids::uuid_random_generator gen{generator};

  uuids::uuid id = gen();
  return uuids::to_string(id);
}

}  // namespace

std::unique_ptr<Ruleset> CreateRuleset(const std::string& name) {
  if (name == "standard") {
    return std::make_unique<StandardRuleset>();
  }

  return nullptr;
}

void PrintGame(const GameState& game, bool render, bool redraw_mode,
               const std::map<SnakeId, char>& snake_head_syms) {
  if (!redraw_mode) {
    nlohmann::json json = CreateJson(game);
    std::cout << json.dump() << std::endl;
  } else {
    std::cout << "\033[2J\033[H";
  }

  if (render || redraw_mode) {
    std::cout << RenderGame(game, snake_head_syms);
  }
}

std::tuple<std::map<SnakeId, Move>, std::map<SnakeId, int>> GetMoves(
    const GameState& game,
    const std::unordered_map<
        SnakeId, std::unique_ptr<battlesnake::interface::Battlesnake>>&
        snakes) {
  std::map<SnakeId, Move> moves;
  std::map<SnakeId, int> latencies;

  // TODO: send requests in parallel

  for (const Snake& snake : game.board.snakes) {
    if (snake.IsEliminated()) {
      // Move only non-eliminated snakes.
      continue;
    }

    GameState game_for_snake = game;
    game_for_snake.you = snake;
    SnakeId id = snake.id;

    auto snake_it = snakes.find(id);
    if (snake_it == snakes.end()) {
      continue;
    }

    const std::unique_ptr<Battlesnake>& snake_interface = snake_it->second;
    auto start = std::chrono::high_resolution_clock::now();
    moves[id] = snake_interface->Move(game_for_snake);
    auto end = std::chrono::high_resolution_clock::now();
    latencies[id] =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
  }
  return std::tie(moves, latencies);
}

void StartAll(
    const GameState& game,
    const std::unordered_map<
        SnakeId, std::unique_ptr<battlesnake::interface::Battlesnake>>&
        snakes) {
  for (const Snake& snake : game.board.snakes) {
    GameState game_for_snake = game;
    game_for_snake.you = snake;
    SnakeId id = snake.id;

    auto snake_it = snakes.find(id);
    if (snake_it == snakes.end()) {
      continue;
    }

    const std::unique_ptr<Battlesnake>& snake_interface = snake_it->second;
    snake_interface->Start(game_for_snake);
  }
}

void EndAll(const GameState& game,
            const std::unordered_map<
                SnakeId, std::unique_ptr<battlesnake::interface::Battlesnake>>&
                snakes) {
  for (const Snake& snake : game.board.snakes) {
    GameState game_for_snake = game;
    game_for_snake.you = snake;
    SnakeId id = snake.id;

    auto snake_it = snakes.find(id);
    if (snake_it == snakes.end()) {
      continue;
    }

    const std::unique_ptr<Battlesnake>& snake_interface = snake_it->second;
    snake_interface->End(game_for_snake);
  }
}

int PlayGame(const CliOptions& options) {
  CurlInit curl_init;

  std::unique_ptr<Ruleset> ruleset = CreateRuleset(options.gametype);
  if (ruleset == nullptr) {
    std::cerr << "Unknown game type: " << options.gametype << std::endl;
    return 10;
  }

  std::unordered_map<SnakeId,
                     std::unique_ptr<battlesnake::interface::Battlesnake>>
      snakes;
  std::unordered_map<SnakeId, std::string> names;
  std::vector<SnakeId> ids;

  for (const SnakeNameUrl& name_url : options.snakes) {
    std::string id = GenerateId();
    ids.push_back(id);
    snakes[id] = std::make_unique<HttpClientBattlesnake>(name_url.url);
    names[id] = name_url.name;
  }

  GameState game{
      .game =
          GameInfo{
              .id = GenerateId(),
              .ruleset{
                  .name = options.gametype,
                  .version = "v0.0.1",
              },
              .timeout = 500,
          },
      .board =
          ruleset->CreateInitialBoardState(options.width, options.height, ids),
  };

  std::map<SnakeId, char> snake_head_syms;
  char head_sym = 'A';

  for (Snake& snake : game.board.snakes) {
    snake.name = names[snake.id];
    snake_head_syms[snake.id] = head_sym;
    ++head_sym;
  }

  PrintGame(game, options.view_map, options.view_map_only, snake_head_syms);
  StartAll(game, snakes);

  for (game.turn = 0; !ruleset->IsGameOver(game.board); ++game.turn) {
    PrintGame(game, options.view_map, options.view_map_only, snake_head_syms);
    std::map<SnakeId, Move> moves;
    std::map<SnakeId, int> latencies;
    std::tie(moves, latencies) = GetMoves(game, snakes);
    game.board = ruleset->CreateNextBoardState(game.board, moves);

    for (Snake& snake : game.board.snakes) {
      auto latency_it = latencies.find(snake.id);
      if (latency_it == latencies.end()) {
        snake.latency = "0";
      } else {
        snake.latency = std::to_string(latency_it->second);
      }
    }
  }

  PrintGame(game, options.view_map, options.view_map_only, snake_head_syms);
  EndAll(game, snakes);

  return 0;
}

}  // namespace cli
}  // namespace battlesnake
