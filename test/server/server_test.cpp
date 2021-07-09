#include "battlesnake/server/server.h"

#include <chrono>
#include <memory>
#include <thread>

#include "battlesnake/interface/battlesnake.h"
#include "battlesnake/json/converter.h"
#include "client_http.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

namespace battlesnake {
namespace server {

namespace {

using ::testing::_;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::NiceMock;
using ::testing::Pointee;
using ::testing::Return;

using namespace battlesnake::rules;
using namespace battlesnake::interface;
using namespace battlesnake::json;

using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

constexpr int kPortNumber = 18888;
constexpr int kThreadsCount = 2;

class TestBattlesnake : public Battlesnake {
 public:
  MOCK_METHOD(Customization, GetCustomization, ());
  MOCK_METHOD(void, Start, (const GameState& game_state));
  MOCK_METHOD(void, End, (const GameState& game_state));
  MOCK_METHOD(MoveResponse, Move, (const GameState& game_state));
};

std::string Http(const std::string& path, const std::string& method,
                 const std::string& content) {
  HttpClient client("localhost:" + std::to_string(kPortNumber));
  auto r = client.request(method, path, content);
  std::stringstream ss;
  ss << r->content.rdbuf();
  return ss.str();
}

std::string Get(const std::string& path) { return Http(path, "GET", ""); }

std::string Post(const std::string& path, const std::string& content) {
  return Http(path, "POST", content);
}

GameState CreateGameState() {
  return GameState{
      .game{
          .id = "totally-unique-game-id",
          .ruleset{.name = "standard", .version = "v1.2.3"},
          .timeout = 500,
      },
      .turn = 987,
      .board{.width = 5, .height = 15},
      .you{
          .id = "snake_id",
          .body =
              {
                  Point{10, 1},
                  Point{10, 2},
                  Point{10, 3},
              },
          .health = 75,
          .name = "Test Caterpillar",
          .latency = "123",
          .shout = "Why are we shouting???",
          .squad = "The Suicide Squad",
      },
  };
}

// -----------------------------------------------------------------------------

class ServerTest : public testing::Test {};

TEST_F(ServerTest, Construct) {
  NiceMock<TestBattlesnake> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
}

TEST_F(ServerTest, RunAndStop) {
  NiceMock<TestBattlesnake> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  server.Stop();
  server_thread->join();
}

TEST_F(ServerTest, GetCustomization) {
  testing::NiceMock<TestBattlesnake> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  Customization expected_customization{
      .apiversion = "api_ver",
      .author = "a",
      .color = "#123456",
      .head = "h",
      .tail = "t",
      .version = "v",
  };

  EXPECT_CALL(battlesnake, GetCustomization())
      .WillOnce(Return(expected_customization));

  std::string customization_str = Get("/");
  auto customization =
      ParseJsonCustomization(nlohmann::json::parse(customization_str));

  server.Stop();
  server_thread->join();

  EXPECT_THAT(customization.apiversion, Eq(expected_customization.apiversion));
  EXPECT_THAT(customization.author, Eq(expected_customization.author));
  EXPECT_THAT(customization.color, Eq(expected_customization.color));
  EXPECT_THAT(customization.head, Eq(expected_customization.head));
  EXPECT_THAT(customization.tail, Eq(expected_customization.tail));
  EXPECT_THAT(customization.version, Eq(expected_customization.version));
}

TEST_F(ServerTest, Start) {
  testing::NiceMock<TestBattlesnake> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  auto game = CreateGameState();
  GameState game_received;
  EXPECT_CALL(battlesnake, Start(_)).WillOnce([&](const GameState& game_state) {
    game_received = game_state;
  });

  // Don't care about actual response.
  Post("/start", CreateJson(game).dump());

  server.Stop();
  server_thread->join();

  EXPECT_THAT(game_received.game.id, Eq(game.game.id));
}

TEST_F(ServerTest, End) {
  testing::NiceMock<TestBattlesnake> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  auto game = CreateGameState();
  GameState game_received;
  EXPECT_CALL(battlesnake, End(_)).WillOnce([&](const GameState& game_state) {
    game_received = game_state;
  });

  // Don't care about actual response.
  Post("/end", CreateJson(game).dump());

  server.Stop();
  server_thread->join();

  EXPECT_THAT(game_received.game.id, Eq(game.game.id));
}

TEST_F(ServerTest, Move) {
  testing::NiceMock<TestBattlesnake> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  auto game = CreateGameState();
  GameState game_received;
  EXPECT_CALL(battlesnake, Move(_))
      .WillOnce([&](const GameState& game_state) -> Battlesnake::MoveResponse {
        game_received = game_state;
        return Battlesnake::MoveResponse{
            .move = Move::Left,
            .shout = "Why are we shouting???",
        };
      });

  auto response = nlohmann::json::parse(Post("/move", CreateJson(game).dump()));

  server.Stop();
  server_thread->join();

  EXPECT_THAT(game_received.game.id, Eq(game.game.id));
  EXPECT_THAT(response["move"], Eq("left"));
  EXPECT_THAT(response["shout"], Eq("Why are we shouting???"));
}

}  // namespace
}  // namespace server
}  // namespace battlesnake
