#include <curl/curl.h>
#include <uuid.h>

#include <chrono>
#include <future>
#include <memory>
#include <unordered_map>

#include "battlesnake/json/converter.h"
#include "battlesnake/player/game_player.h"
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
using namespace ::battlesnake::player;

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

}  // namespace

int PlayGame(const CliOptions& options) {
  CurlInit curl_init;

  std::unique_ptr<Ruleset> ruleset = CreateRuleset(options.gametype);
  if (ruleset == nullptr) {
    std::cerr << "Unknown game type: " << options.gametype << std::endl;
    return 10;
  }

  GamePlayer player;
  player.SetGameId(GenerateId());
  player.SetRuleset(ruleset.get(), options.gametype, options.timeout);
  player.SetBoardSize(options.width, options.height);
  player.SetRequestsMode(options.sequential_http ? RequestsMode::Sequential
                                                 : RequestsMode::Parallel);
  if (options.view_map_only) {
    player.SetPrintMode(PrintMode::MapOnly);
  } else if (options.view_map) {
    player.SetPrintMode(PrintMode::StateAndMap);
  } else {
    player.SetPrintMode(PrintMode::StateOnly);
  }

  std::unordered_map<std::string, std::string> names;
  std::vector<std::unique_ptr<HttpClientBattlesnake>> battlesnakes;

  std::vector<std::string> squads = {"red", "blue"};
  int current_squad = 0;
  for (const SnakeNameUrl& name_url : options.snakes) {
    std::string id = GenerateId();
    names[id] = name_url.name;

    auto battlesnake = std::make_unique<HttpClientBattlesnake>(name_url.url);
    player.AddBattlesnake(id, battlesnake.get(), name_url.name,
                          squads[current_squad]);
    current_squad = (current_squad + 1) % squads.size();

    battlesnakes.push_back(std::move(battlesnake));
  }

  player.Play();

  for (SnakeId id : player.Winners()) {
    std::cout << "Winner: " << names[std::string(id)] << std::endl;
  }

  return 0;
}

}  // namespace cli
}  // namespace battlesnake
