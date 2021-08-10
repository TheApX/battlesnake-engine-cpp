#pragma once

#include <string>
#include <unordered_map>

#include "battlesnake/interface/battlesnake.h"
#include "battlesnake/rules/ruleset.h"

namespace battlesnake {
namespace player {

enum class PrintMode {
  DoNotPrint,
  StateOnly,
  MapOnly,
  StateAndMap,
};

enum class RequestsMode {
  Parallel,
  Sequential,
};

class GamePlayer {
 public:
  void SetGameId(const std::string& game_id);
  void SetRuleset(battlesnake::rules::Ruleset* ruleset,
                  const std::string& gametype_name, int timeout);
  void SetBoardSize(int width, int height);
  void AddBattlesnake(const std::string& id,
                      battlesnake::interface::Battlesnake* battlesnake,
                      const std::string& name = std::string(),
                      const std::string& squad = std::string());

  void SetPrintMode(PrintMode mode);
  void SetRequestsMode(RequestsMode mode);

  void Play();

  const std::vector<battlesnake::rules::SnakeId>& Winners() const;

 private:
  struct PlayerInfo {
    std::string id;
    battlesnake::interface::Battlesnake* battlesnake;
    std::string name;
    std::string squad;
  };

  std::string game_id_;
  battlesnake::rules::Ruleset* ruleset_ = nullptr;
  std::string gametype_name_ = "standard";
  int timeout_ = 500;
  int width_ = battlesnake::rules::kBoardSizeMedium;
  int height_ = battlesnake::rules::kBoardSizeMedium;
  std::vector<PlayerInfo> players_;
  PrintMode print_mode_ = PrintMode::DoNotPrint;
  RequestsMode requests_mode_ = RequestsMode::Parallel;

  std::shared_ptr<battlesnake::rules::StringPool> string_pool_ =
      std::make_shared<battlesnake::rules::StringPool>();
  std::unordered_map<battlesnake::rules::SnakeId,
                     battlesnake::interface::Battlesnake*>
      snakes_map_;

  std::vector<battlesnake::rules::SnakeId> winners_;

  void PrintGame(const battlesnake::rules::GameState& game,
                 const std::unordered_map<battlesnake::rules::SnakeId, char>&
                     snake_head_syms) const;

  void StartAll(const battlesnake::rules::GameState& game);
  void EndAll(const battlesnake::rules::GameState& game);

  using GetMovesResult = std::tuple<
      std::unordered_map<battlesnake::rules::SnakeId,
                         battlesnake::interface::Battlesnake::MoveResponse>,
      std::unordered_map<battlesnake::rules::SnakeId, int>>;

  GetMovesResult GetMovesParallel(const battlesnake::rules::GameState& game);
  GetMovesResult GetMovesSequential(const battlesnake::rules::GameState& game);
  GetMovesResult GetMoves(const battlesnake::rules::GameState& game);
};

}  // namespace player
}  // namespace battlesnake
