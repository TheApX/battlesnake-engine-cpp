#include <curl/curl.h>
#include <uuid.h>

#include <chrono>
#include <future>
#include <memory>
#include <unordered_map>

#include "battlesnake/json/converter.h"
#include "battlesnake/rules/constrictor_ruleset.h"
#include "battlesnake/rules/helpers.h"
#include "battlesnake/rules/royale_ruleset.h"
#include "battlesnake/rules/ruleset.h"
#include "battlesnake/rules/solo_ruleset.h"
#include "battlesnake/rules/squad_ruleset.h"
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

  if (name == "solo") {
    return std::make_unique<SoloRuleset>();
  }

  if (name == "royale") {
    return std::make_unique<RoyaleRuleset>();
  }

  if (name == "constrictor") {
    return std::make_unique<ConstrictorRuleset>();
  }

  if (name == "squad") {
    return std::make_unique<SquadRuleset>();
  }

  return nullptr;
}

void PrintGame(const GameState& game, bool render, bool redraw_mode,
               const std::unordered_map<SnakeId, char>& snake_head_syms) {
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

struct MoveResult {
  SnakeId snake_id;
  Battlesnake::MoveResponse response;
  int latency;
};

// This is not completely async because it waits for snake->Move(,,) to return,
// but it's the same that the server does. And latency is measured until
// the response. It is completely async if the battlesnake returns immediately
// and then responds from a different thread.
MoveResult MoveSnake(std::shared_ptr<StringPool> pool, const GameState& game,
                     const Snake& snake,
                     battlesnake::interface::Battlesnake* snake_interface) {
  MoveResult move_result;

  if (snake.IsEliminated()) {
    // Move only non-eliminated snakes.
    return move_result;
  }

  if (snake_interface == nullptr) {
    // No snake interface.
    return move_result;
  }

  GameState game_for_snake = game;
  game_for_snake.you = snake;

  move_result.snake_id = snake.id;
  std::promise<void> has_result;

  auto start = std::chrono::high_resolution_clock::now();
  snake_interface->Move(
      pool, game_for_snake,
      [&move_result, &start,
       &has_result](const Battlesnake::MoveResponse& result) {
        auto end = std::chrono::high_resolution_clock::now();
        move_result.response = result;
        move_result.latency =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();
        has_result.set_value();
      });

  has_result.get_future().wait();
  return move_result;
}

std::tuple<std::unordered_map<SnakeId, Battlesnake::MoveResponse>,
           std::unordered_map<SnakeId, int>>
GetMovesParallel(
    std::shared_ptr<StringPool> pool, const GameState& game,
    const std::unordered_map<SnakeId, std::unique_ptr<Battlesnake>>& snakes) {
  std::unordered_map<SnakeId, Battlesnake::MoveResponse> move_responses;
  std::unordered_map<SnakeId, int> latencies;

  // Send requests in parallel.
  std::vector<std::future<MoveResult>> move_futures;
  for (const Snake& snake : game.board.snakes) {
    if (snake.IsEliminated()) {
      // Move only non-eliminated snakes.
      continue;
    }

    SnakeId id = snake.id;
    auto snake_it = snakes.find(id);
    if (snake_it == snakes.end()) {
      continue;
    }

    move_futures.push_back(std::async(std::launch::async, MoveSnake, pool, game,
                                      snake, snake_it->second.get()));
  }

  // Wait for and process responses.
  for (std::future<MoveResult>& move_future : move_futures) {
    MoveResult move_result = move_future.get();
    if (move_result.snake_id.empty()) {
      continue;
    }

    move_responses[move_result.snake_id] = move_result.response;
    latencies[move_result.snake_id] = move_result.latency;
  }

  return std::tie(move_responses, latencies);
}

std::tuple<std::unordered_map<SnakeId, Battlesnake::MoveResponse>,
           std::unordered_map<SnakeId, int>>
GetMovesSequential(
    std::shared_ptr<StringPool> pool, const GameState& game,
    const std::unordered_map<SnakeId, std::unique_ptr<Battlesnake>>& snakes) {
  std::unordered_map<SnakeId, Battlesnake::MoveResponse> move_responses;
  std::unordered_map<SnakeId, int> latencies;

  // Send requests in parallel.
  std::vector<std::future<MoveResult>> move_futures;
  for (const Snake& snake : game.board.snakes) {
    if (snake.IsEliminated()) {
      // Move only non-eliminated snakes.
      continue;
    }

    SnakeId id = snake.id;
    auto snake_it = snakes.find(id);
    if (snake_it == snakes.end()) {
      continue;
    }

    MoveResult move_result =
        MoveSnake(pool, game, snake, snake_it->second.get());
    if (move_result.snake_id.empty()) {
      continue;
    }

    move_responses[move_result.snake_id] = move_result.response;
    latencies[move_result.snake_id] = move_result.latency;
  }

  return std::tie(move_responses, latencies);
}

std::tuple<std::unordered_map<SnakeId, Battlesnake::MoveResponse>,
           std::unordered_map<SnakeId, int>>
GetMoves(
    std::shared_ptr<StringPool> pool, const GameState& game,
    const std::unordered_map<SnakeId, std::unique_ptr<Battlesnake>>& snakes,
    bool sequential_http) {
  if (sequential_http) {
    return GetMovesSequential(pool, game, snakes);
  } else {
    return GetMovesParallel(pool, game, snakes);
  }
}

void StartAll(
    std::shared_ptr<StringPool> pool, const GameState& game,
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
    snake_interface->Start(pool, game_for_snake, []() {});
  }
}

void EndAll(std::shared_ptr<StringPool> pool, const GameState& game,
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
    snake_interface->End(pool, game_for_snake, []() {});
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
  std::shared_ptr<StringPool> pool = std::make_shared<StringPool>();

  for (const SnakeNameUrl& name_url : options.snakes) {
    SnakeId id = pool->Add(GenerateId());
    ids.push_back(id);
    snakes[id] = std::make_unique<HttpClientBattlesnake>(name_url.url);
    names[id] = pool->Add(name_url.name);
  }

  GameState game{
      .game =
          GameInfo{
              .id = pool->Add(GenerateId()),
              .ruleset{
                  .name = pool->Add(options.gametype),
                  .version = pool->Add("v0.0.1"),
              },
              .timeout = options.timeout,
          },
      .board =
          ruleset->CreateInitialBoardState(options.width, options.height, ids),
  };

  std::vector<std::string> squads = {"red", "blue"};
  if (options.gametype == "squad") {
    for (int i = 0; i < game.board.snakes.size(); ++i) {
      Snake& snake = game.board.snakes[i];
      snake.squad = squads[i % 2];
    }
  }

  std::unordered_map<SnakeId, char> snake_head_syms;
  char head_sym = 'A';

  for (Snake& snake : game.board.snakes) {
    snake.name = names[snake.id];
    snake_head_syms[snake.id] = head_sym;
    ++head_sym;
  }

  PrintGame(game, options.view_map, options.view_map_only, snake_head_syms);
  StartAll(pool, game, snakes);

  int total_latency = 0;
  for (game.turn = 1; !ruleset->IsGameOver(game.board); ++game.turn) {
    PrintGame(game, options.view_map, options.view_map_only, snake_head_syms);
    std::cout << "Total latency: " << total_latency << std::endl;

    std::unordered_map<SnakeId, Battlesnake::MoveResponse> move_responses;
    std::unordered_map<SnakeId, int> latencies;

    auto start = std::chrono::high_resolution_clock::now();
    std::tie(move_responses, latencies) =
        GetMoves(pool, game, snakes, options.sequential_http);
    auto end = std::chrono::high_resolution_clock::now();
    total_latency =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();

    std::unordered_map<SnakeId, Move> moves;
    for (const auto& [id, response] : move_responses) {
      moves[id] = response.move;
    }

    game.board = ruleset->CreateNextBoardState(game.board, moves, game.turn);

    for (Snake& snake : game.board.snakes) {
      auto latency_it = latencies.find(snake.id);
      if (latency_it == latencies.end()) {
        snake.latency = pool->Add("0");
      } else {
        snake.latency = pool->Add(std::to_string(latency_it->second));
      }

      auto move_response_it = move_responses.find(snake.id);
      if (move_response_it == move_responses.end()) {
        snake.shout = pool->Add("");
      } else {
        snake.shout = pool->Add(move_response_it->second.shout);
      }
    }
  }

  PrintGame(game, options.view_map, options.view_map_only, snake_head_syms);
  EndAll(pool, game, snakes);

  for (const Snake& snake : game.board.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }

    std::cout << "Winner: " << snake.name << std::endl;
  }

  return 0;
}

}  // namespace cli
}  // namespace battlesnake
