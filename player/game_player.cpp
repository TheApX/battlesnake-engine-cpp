#include "battlesnake/player/game_player.h"

#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

#include "battlesnake/json/converter.h"
#include "battlesnake/rules/helpers.h"

namespace battlesnake {
namespace player {

namespace {

using namespace ::battlesnake::json;
using namespace ::battlesnake::interface;
using namespace ::battlesnake::rules;

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

}  // namespace

void GamePlayer::SetGameId(const std::string& game_id) { game_id_ = game_id; }

void GamePlayer::SetRuleset(battlesnake::rules::Ruleset* ruleset,
                            const std::string& gametype_name, int timeout) {
  ruleset_ = ruleset;
  gametype_name_ = gametype_name;
  timeout_ = timeout;
}

void GamePlayer::SetBoardSize(int width, int height) {
  width_ = width;
  height_ = height;
}

void GamePlayer::AddBattlesnake(
    const std::string& id, battlesnake::interface::Battlesnake* battlesnake,
    const std::string& name, const std::string& squad) {
  players_.emplace_back(PlayerInfo{
      .id = id,
      .battlesnake = battlesnake,
      .name = name.empty() ? id : name,
      .squad = squad,
  });
}

void GamePlayer::SetPrintMode(PrintMode mode) { print_mode_ = mode; }

void GamePlayer::SetRequestsMode(RequestsMode mode) { requests_mode_ = mode; }

void GamePlayer::Play() {
  std::unordered_map<SnakeId, std::string> names;

  std::vector<SnakeId> snake_ids;
  for (const PlayerInfo& player : players_) {
    snake_ids.push_back(player.id);
    snakes_map_[player.id] = player.battlesnake;
    names[player.id] = string_pool_->Add(player.name);
  }

  GameState game{
      .game =
          GameInfo{
              .id = string_pool_->Add(game_id_),
              .ruleset{
                  .name = string_pool_->Add(gametype_name_),
                  .version = string_pool_->Add("v0.0.1"),
              },
              .timeout = timeout_,
          },
      .board = ruleset_->CreateInitialBoardState(width_, height_, snake_ids),
  };

  std::unordered_map<SnakeId, char> snake_head_syms;
  char head_sym = 'A';

  for (Snake& snake : game.board.snakes) {
    snake.name = names[snake.id];
    snake_head_syms[snake.id] = head_sym;
    ++head_sym;
  }

  PrintGame(game, snake_head_syms);
  StartAll(game);

  //   int total_latency = 0;
  for (game.turn = 1; !ruleset_->IsGameOver(game.board); ++game.turn) {
    PrintGame(game, snake_head_syms);

    std::unordered_map<SnakeId, Battlesnake::MoveResponse> move_responses;
    std::unordered_map<SnakeId, int> latencies;

    auto start = std::chrono::high_resolution_clock::now();
    std::tie(move_responses, latencies) = GetMoves(game);
    auto end = std::chrono::high_resolution_clock::now();
    // total_latency =
    //     std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
    //         .count();

    std::unordered_map<SnakeId, Move> moves;
    for (const auto& [id, response] : move_responses) {
      moves[id] = response.move;
    }

    game.board = ruleset_->CreateNextBoardState(game.board, moves, game.turn);

    for (Snake& snake : game.board.snakes) {
      auto latency_it = latencies.find(snake.id);
      if (latency_it == latencies.end()) {
        snake.latency = string_pool_->Add("0");
      } else {
        snake.latency = string_pool_->Add(std::to_string(latency_it->second));
      }

      auto move_response_it = move_responses.find(snake.id);
      if (move_response_it == move_responses.end()) {
        snake.shout = string_pool_->Add("");
      } else {
        snake.shout = string_pool_->Add(move_response_it->second.shout);
      }
    }
  }

  PrintGame(game, snake_head_syms);
  EndAll(game);

  for (const Snake& snake : game.board.snakes) {
    if (snake.IsEliminated()) {
      continue;
    }

    winners_.push_back(snake.id);
  }
}

const std::vector<SnakeId>& GamePlayer::Winners() const { return winners_; }

void GamePlayer::PrintGame(
    const GameState& game,
    const std::unordered_map<SnakeId, char>& snake_head_syms) const {
  if (print_mode_ == PrintMode::DoNotPrint) {
    return;
  }

  if (print_mode_ == PrintMode::MapOnly) {
    std::cout << "\033[2J\033[H";
  } else {
    nlohmann::json json = CreateJson(game);
    std::cout << json.dump() << std::endl;
  }

  if (print_mode_ != PrintMode::StateOnly) {
    std::cout << RenderGame(game, snake_head_syms);
  }
}

void GamePlayer::StartAll(const GameState& game) {
  for (const Snake& snake : game.board.snakes) {
    GameState game_for_snake = game;
    game_for_snake.you = snake;
    SnakeId id = snake.id;

    auto snake_it = snakes_map_.find(id);
    if (snake_it == snakes_map_.end()) {
      continue;
    }

    Battlesnake* snake_interface = snake_it->second;
    snake_interface->Start(string_pool_, game_for_snake, []() {});
  }
}

void GamePlayer::EndAll(const GameState& game) {
  for (const Snake& snake : game.board.snakes) {
    GameState game_for_snake = game;
    game_for_snake.you = snake;
    SnakeId id = snake.id;

    auto snake_it = snakes_map_.find(id);
    if (snake_it == snakes_map_.end()) {
      continue;
    }

    Battlesnake* snake_interface = snake_it->second;
    snake_interface->End(string_pool_, game_for_snake, []() {});
  }
}

GamePlayer::GetMovesResult GamePlayer::GetMovesParallel(const GameState& game) {
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
    auto snake_it = snakes_map_.find(id);
    if (snake_it == snakes_map_.end()) {
      continue;
    }

    move_futures.push_back(std::async(std::launch::async, MoveSnake,
                                      string_pool_, game, snake,
                                      snake_it->second));
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

GamePlayer::GetMovesResult GamePlayer::GetMovesSequential(
    const GameState& game) {
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
    auto snake_it = snakes_map_.find(id);
    if (snake_it == snakes_map_.end()) {
      continue;
    }

    MoveResult move_result =
        MoveSnake(string_pool_, game, snake, snake_it->second);
    if (move_result.snake_id.empty()) {
      continue;
    }

    move_responses[move_result.snake_id] = move_result.response;
    latencies[move_result.snake_id] = move_result.latency;
  }

  return std::tie(move_responses, latencies);
}

GamePlayer::GetMovesResult GamePlayer::GetMoves(const GameState& game) {
  switch (requests_mode_) {
    case RequestsMode::Sequential:
      return GetMovesSequential(game);

    case RequestsMode::Parallel:
    default:
      return GetMovesParallel(game);
  }
}

}  // namespace player
}  // namespace battlesnake
