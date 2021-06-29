#include <battlesnake/json/converter.h>
#include <battlesnake/server/server.h>

#include <server_http.hpp>

namespace battlesnake {
namespace server {

namespace {

using HttpServer = ::SimpleWeb::Server<SimpleWeb::HTTP>;
using namespace ::battlesnake::interface;
using namespace ::battlesnake::rules;

}  // namespace

class BattlesnakeServer::BattlesnakeServerImpl {
 public:
  BattlesnakeServerImpl(Battlesnake* battlesnake, int port, int threads);

  ~BattlesnakeServerImpl();

  void Run(const std::function<void(unsigned short /*port*/)>& callback);
  void Stop();

 private:
  HttpServer server_;
  int port_ = 0;
  int threads_ = 0;
  Battlesnake* battlesnake_ = nullptr;

  void onInfo(std::shared_ptr<HttpServer::Response> response,
              std::shared_ptr<HttpServer::Request> request);
  void onStart(std::shared_ptr<HttpServer::Response> response,
               std::shared_ptr<HttpServer::Request> request);
  void onEnd(std::shared_ptr<HttpServer::Response> response,
             std::shared_ptr<HttpServer::Request> request);
  void onMove(std::shared_ptr<HttpServer::Response> response,
              std::shared_ptr<HttpServer::Request> request);
};

BattlesnakeServer::BattlesnakeServerImpl::BattlesnakeServerImpl(
    Battlesnake* battlesnake, int port, int threads)
    : battlesnake_(battlesnake), port_(port), threads_(threads) {
  server_.config.port = port_;
  server_.config.thread_pool_size = threads;

  server_.default_resource["GET"] =
      [&](std::shared_ptr<HttpServer::Response> response,
          std::shared_ptr<HttpServer::Request> request) {
        this->onInfo(response, request);
      };
  server_.resource["/start"]["POST"] =
      [&](std::shared_ptr<HttpServer::Response> response,
          std::shared_ptr<HttpServer::Request> request) {
        this->onStart(response, request);
      };
  server_.resource["/end"]["POST"] =
      [&](std::shared_ptr<HttpServer::Response> response,
          std::shared_ptr<HttpServer::Request> request) {
        this->onEnd(response, request);
      };
  server_.resource["/move"]["POST"] =
      [&](std::shared_ptr<HttpServer::Response> response,
          std::shared_ptr<HttpServer::Request> request) {
        this->onMove(response, request);
      };
}

BattlesnakeServer::BattlesnakeServerImpl::~BattlesnakeServerImpl() {
  // Make sure the server is stopped.
  Stop();
}

void BattlesnakeServer::BattlesnakeServerImpl::Run(
    const std::function<void(unsigned short /*port*/)>& callback) {
  server_.start(callback);
}

void BattlesnakeServer::BattlesnakeServerImpl::Stop() {
  //
  server_.stop();
}

void BattlesnakeServer::BattlesnakeServerImpl::onInfo(
    std::shared_ptr<HttpServer::Response> response,
    std::shared_ptr<HttpServer::Request> request) {
  try {
    auto customization = battlesnake_->GetCustomization();
    response->write(battlesnake::json::CreateJson(customization).dump());
  } catch (std::exception) {
    response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                    "Internal server error");
  }
}

void BattlesnakeServer::BattlesnakeServerImpl::onStart(
    std::shared_ptr<HttpServer::Response> response,
    std::shared_ptr<HttpServer::Request> request) {
  try {
    auto content = request->content.string();
    auto game_state =
        battlesnake::json::ParseJsonGameState(nlohmann::json::parse(content));
    battlesnake_->Start(game_state);
    response->write("ok");
  } catch (std::exception) {
    response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                    "Internal server error");
  }
}

void BattlesnakeServer::BattlesnakeServerImpl::onEnd(
    std::shared_ptr<HttpServer::Response> response,
    std::shared_ptr<HttpServer::Request> request) {
  try {
    auto content = request->content.string();
    auto game_state =
        battlesnake::json::ParseJsonGameState(nlohmann::json::parse(content));
    battlesnake_->End(game_state);
    response->write("ok");
  } catch (std::exception) {
    response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                    "Internal server error");
  }
}

void BattlesnakeServer::BattlesnakeServerImpl::onMove(
    std::shared_ptr<HttpServer::Response> response,
    std::shared_ptr<HttpServer::Request> request) {
  try {
    auto content = request->content.string();
    auto game_state =
        battlesnake::json::ParseJsonGameState(nlohmann::json::parse(content));

    auto move = battlesnake_->Move(game_state);
    nlohmann::json json{{"shout", move.shout}};

    switch (move.move) {
      case Move::Up:
        json["move"] = "up";
        break;
      case Move::Down:
        json["move"] = "down";
        break;
      case Move::Left:
        json["move"] = "left";
        break;
      case Move::Right:
        json["move"] = "right";
        break;

      default:
        break;
    }

    response->write(json.dump());
  } catch (std::exception) {
    response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                    "Internal server error");
  }
}

// -----------------------------------------------------------------------------

BattlesnakeServer::BattlesnakeServer(Battlesnake* battlesnake, int port,
                                     int threads)
    : impl(std::make_unique<BattlesnakeServerImpl>(battlesnake, port,
                                                   threads)) {}

BattlesnakeServer::~BattlesnakeServer() {}

void BattlesnakeServer::Run(
    const std::function<void(unsigned short /*port*/)>& callback) {
  impl->Run(callback);
}
void BattlesnakeServer::Stop() { impl->Stop(); }

std::unique_ptr<std::thread> BattlesnakeServer::RunOnNewThread() {
  std::promise<unsigned short> server_port;

  auto result = std::make_unique<std::thread>([&]() {
    this->Run(
        [&server_port](unsigned short port) { server_port.set_value(port); });
  });
  server_port.get_future().wait();

  return result;
}

}  // namespace server
}  // namespace battlesnake
