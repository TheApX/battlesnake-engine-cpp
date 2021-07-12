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
using ::testing::Ge;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::Lt;
using ::testing::NiceMock;
using ::testing::Pointee;
using ::testing::Return;

using namespace battlesnake::rules;
using namespace battlesnake::interface;
using namespace battlesnake::json;

using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;

constexpr int kPortNumber = 18888;
constexpr int kThreadsCount = 2;

class TestBattlesnakeSync : public Battlesnake {
 public:
  MOCK_METHOD(Customization, GetCustomization, ());
  MOCK_METHOD(void, Start, (const GameState& game_state));
  MOCK_METHOD(void, End, (const GameState& game_state));
  MOCK_METHOD(MoveResponse, Move, (const GameState& game_state));
};

class TestBattlesnakeAsync : public Battlesnake {
 public:
  MOCK_METHOD(
      void, GetCustomization,
      (std::function<void(const battlesnake::rules::Customization& result)>
           respond));
  MOCK_METHOD(void, Start,
              (std::shared_ptr<battlesnake::rules::StringPool> string_pool,
               const GameState& game_state, std::function<void()> respond));
  MOCK_METHOD(void, End,
              (std::shared_ptr<battlesnake::rules::StringPool> string_pool,
               const GameState& game_state, std::function<void()> respond));
  MOCK_METHOD(void, Move,
              (std::shared_ptr<battlesnake::rules::StringPool> string_pool,
               const GameState& game_state,
               std::function<void(const MoveResponse& result)> respond));
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

GameState CreateGameState(StringPool& pool) {
  return GameState{
      .game{
          .id = pool.Add("totally-unique-game-id"),
          .ruleset{.name = pool.Add("standard"), .version = pool.Add("v1.2.3")},
          .timeout = 500,
      },
      .turn = 987,
      .board{.width = 5, .height = 15},
      .you{
          .id = pool.Add("snake_id"),
          .body =
              {
                  Point{10, 1},
                  Point{10, 2},
                  Point{10, 3},
              },
          .health = 75,
          .name = pool.Add("Test Caterpillar"),
          .latency = pool.Add("123"),
          .shout = pool.Add("Why are we shouting???"),
          .squad = pool.Add("The Suicide Squad"),
      },
  };
}

// -----------------------------------------------------------------------------

class ServerTestSync : public testing::Test {};

TEST_F(ServerTestSync, Construct) {
  NiceMock<TestBattlesnakeSync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
}

TEST_F(ServerTestSync, RunAndStop) {
  NiceMock<TestBattlesnakeSync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  server.Stop();
  server_thread->join();
}

TEST_F(ServerTestSync, GetCustomization) {
  testing::NiceMock<TestBattlesnakeSync> battlesnake;
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

TEST_F(ServerTestSync, Start) {
  testing::NiceMock<TestBattlesnakeSync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  StringPool pool;
  auto game = CreateGameState(pool);
  std::string received_game_id;
  EXPECT_CALL(battlesnake, Start(_)).WillOnce([&](const GameState& game_state) {
    received_game_id = game_state.game.id;
  });

  // Don't care about actual response.
  Post("/start", CreateJson(game).dump());

  server.Stop();
  server_thread->join();

  EXPECT_THAT(received_game_id, Eq(game.game.id));
}

TEST_F(ServerTestSync, End) {
  testing::NiceMock<TestBattlesnakeSync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  StringPool pool;
  auto game = CreateGameState(pool);
  std::string received_game_id;
  EXPECT_CALL(battlesnake, End(_)).WillOnce([&](const GameState& game_state) {
    received_game_id = game_state.game.id;
  });

  // Don't care about actual response.
  Post("/end", CreateJson(game).dump());

  server.Stop();
  server_thread->join();

  EXPECT_THAT(received_game_id, Eq(game.game.id));
}

TEST_F(ServerTestSync, Move) {
  testing::NiceMock<TestBattlesnakeSync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  StringPool pool;
  auto game = CreateGameState(pool);
  std::string received_game_id;
  EXPECT_CALL(battlesnake, Move(_))
      .WillOnce([&](const GameState& game_state) -> Battlesnake::MoveResponse {
        received_game_id = game_state.game.id;

        return Battlesnake::MoveResponse{
            .move = Move::Left,
            .shout = "Why are we shouting???",
        };
      });

  auto response = nlohmann::json::parse(Post("/move", CreateJson(game).dump()));

  server.Stop();
  server_thread->join();

  EXPECT_THAT(received_game_id, Eq(game.game.id));
  EXPECT_THAT(response["move"], Eq("left"));
  EXPECT_THAT(response["shout"], Eq("Why are we shouting???"));
}

// -----------------------------------------------------------------------------

class ServerTestAsync : public testing::Test {
 protected:
  std::chrono::milliseconds post_respond_delay_ =
      std::chrono::milliseconds(250);
};

TEST_F(ServerTestAsync, GetCustomization) {
  testing::NiceMock<TestBattlesnakeAsync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  Customization expected_customization{
      .apiversion = "a_api_ver",
      .author = "a_a",
      .color = "#ABCDEF",
      .head = "a_h",
      .tail = "a_t",
      .version = "a_v",
  };

  std::promise<void> finished;
  auto begin_time = std::chrono::high_resolution_clock::now();

  EXPECT_CALL(battlesnake, GetCustomization(_))
      .WillOnce([&](std::function<void(const Customization& result)> respond)
                    -> void {
        std::thread worker_thread(
            [&finished, this, expected_customization, respond]() {
              respond(expected_customization);

              std::this_thread::sleep_for(this->post_respond_delay_);
              finished.set_value();
            });
        worker_thread.detach();
      });

  auto customization = ParseJsonCustomization(nlohmann::json::parse(Get("/")));

  // Response must be delivered before the worker thread finishes its work. It
  // sleeps for `post_respond_delay`, check that the response was delivered
  // before half of that time.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Lt(post_respond_delay_ / 2));

  // But received_game_id must be available some time later.
  std::future<void> finished_future = finished.get_future();
  finished_future.get();
  // Check that the worker thread took at least as much time as expected, or
  // more.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Ge(post_respond_delay_));

  server.Stop();
  server_thread->join();

  EXPECT_THAT(customization.apiversion, Eq(expected_customization.apiversion));
  EXPECT_THAT(customization.author, Eq(expected_customization.author));
  EXPECT_THAT(customization.color, Eq(expected_customization.color));
  EXPECT_THAT(customization.head, Eq(expected_customization.head));
  EXPECT_THAT(customization.tail, Eq(expected_customization.tail));
  EXPECT_THAT(customization.version, Eq(expected_customization.version));
}

TEST_F(ServerTestAsync, Start) {
  testing::NiceMock<TestBattlesnakeAsync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  StringPool pool;
  auto game = CreateGameState(pool);

  std::promise<std::string> received_game_id;
  auto begin_time = std::chrono::high_resolution_clock::now();

  EXPECT_CALL(battlesnake, Start(_, _, _))
      .WillOnce([&](std::shared_ptr<battlesnake::rules::StringPool> string_pool,
                    const GameState& game_state,
                    std::function<void()> respond) -> void {
        std::thread worker_thread(
            [&received_game_id, this, string_pool, game_state, respond]() {
              respond();

              std::this_thread::sleep_for(this->post_respond_delay_);
              received_game_id.set_value(std::string(game_state.game.id));
            });
        worker_thread.detach();
      });

  Post("/start", CreateJson(game).dump());

  // Response must be delivered before the worker thread finishes its work. It
  // sleeps for `post_respond_delay`, check that the response was delivered
  // before half of that time.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Lt(post_respond_delay_ / 2));

  // But received_game_id must be available some time later.
  std::future<std::string> received_game_id_future =
      received_game_id.get_future();
  EXPECT_THAT(received_game_id_future.get(), Eq(game.game.id));
  // Check that the worker thread took at least as much time as expected, or
  // more.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Ge(post_respond_delay_));

  server.Stop();
  server_thread->join();
}

TEST_F(ServerTestAsync, End) {
  testing::NiceMock<TestBattlesnakeAsync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  StringPool pool;
  auto game = CreateGameState(pool);

  std::promise<std::string> received_game_id;
  auto begin_time = std::chrono::high_resolution_clock::now();

  EXPECT_CALL(battlesnake, End(_, _, _))
      .WillOnce([&](std::shared_ptr<battlesnake::rules::StringPool> string_pool,
                    const GameState& game_state,
                    std::function<void()> respond) -> void {
        std::thread worker_thread(
            [&received_game_id, this, string_pool, game_state, respond]() {
              respond();

              std::this_thread::sleep_for(this->post_respond_delay_);
              received_game_id.set_value(std::string(game_state.game.id));
            });
        worker_thread.detach();
      });

  Post("/end", CreateJson(game).dump());

  // Response must be delivered before the worker thread finishes its work. It
  // sleeps for `post_respond_delay`, check that the response was delivered
  // before half of that time.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Lt(post_respond_delay_ / 2));

  // But received_game_id must be available some time later.
  std::future<std::string> received_game_id_future =
      received_game_id.get_future();
  EXPECT_THAT(received_game_id_future.get(), Eq(game.game.id));
  // Check that the worker thread took at least as much time as expected, or
  // more.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Ge(post_respond_delay_));

  server.Stop();
  server_thread->join();
}

TEST_F(ServerTestAsync, Move) {
  testing::NiceMock<TestBattlesnakeAsync> battlesnake;
  BattlesnakeServer server(&battlesnake, kPortNumber, kThreadsCount);
  auto server_thread = server.RunOnNewThread();

  StringPool pool;
  auto game = CreateGameState(pool);

  std::promise<std::string> received_game_id;
  auto begin_time = std::chrono::high_resolution_clock::now();

  EXPECT_CALL(battlesnake, Move(_, _, _))
      .WillOnce([&](std::shared_ptr<battlesnake::rules::StringPool> string_pool,
                    const GameState& game_state,
                    std::function<void(const Battlesnake::MoveResponse& result)>
                        respond) -> void {
        std::thread worker_thread(
            [&received_game_id, this, string_pool, game_state, respond]() {
              respond(Battlesnake::MoveResponse{
                  .move = Move::Down,
                  .shout = "Why am I so slow???",
              });

              std::this_thread::sleep_for(this->post_respond_delay_);
              received_game_id.set_value(std::string(game_state.game.id));
            });
        worker_thread.detach();
      });

  auto response = nlohmann::json::parse(Post("/move", CreateJson(game).dump()));

  // Response must be delivered before the worker thread finishes its work. It
  // sleeps for `post_respond_delay`, check that the response was delivered
  // before half of that time.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Lt(post_respond_delay_ / 2));

  // But received_game_id must be available some time later.
  std::future<std::string> received_game_id_future =
      received_game_id.get_future();
  EXPECT_THAT(received_game_id_future.get(), Eq(game.game.id));
  // Check that the worker thread took at least as much time as expected, or
  // more.
  EXPECT_THAT(std::chrono::high_resolution_clock::now() - begin_time,
              Ge(post_respond_delay_));

  server.Stop();
  server_thread->join();

  EXPECT_THAT(response["move"], Eq("down"));
  EXPECT_THAT(response["shout"], Eq("Why am I so slow???"));
}

}  // namespace
}  // namespace server
}  // namespace battlesnake
