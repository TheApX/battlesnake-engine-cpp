#include <client_wss.hpp>
#include <exception>
#include <future>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

#include "options.h"

namespace {

using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

class DownloaderException : public std::exception {
 public:
  explicit DownloaderException(const std::string& error) : error_(error) {}
  const char* what() const noexcept override { return error_.c_str(); }

 private:
  std::string error_;
};

}  // namespace

std::vector<nlohmann::json> DownloadGameData(const DownloaderOptions& options) {
  std::vector<nlohmann::json> result;

  WssClient client("engine.battlesnake.com/socket/" + options.game_id,
                   false);  // Second Client() parameter set to false: no
                            // certificate verification
  client.on_message = [&result](
                          std::shared_ptr<WssClient::Connection> connection,
                          std::shared_ptr<WssClient::InMessage> in_message) {
    auto json = nlohmann::json::parse(in_message->string());
    result.push_back(json);
    std::cout << '.' << std::flush;
  };

  std::promise<bool> finished;
  client.on_close =
      [&finished](std::shared_ptr<WssClient::Connection> /*connection*/,
                  int status,
                  const std::string& /*reason*/) { finished.set_value(true); };

  client.on_error = [&finished](
                        std::shared_ptr<WssClient::Connection> /*connection*/,
                        const SimpleWeb::error_code& ec) {
    std::cout << std::endl;
    std::cout << "Client: Error: " << ec << ", error message: " << ec.message()
              << std::endl;
    finished.set_value(false);
  };

  std::cout << "Downloading";

  client.start();

  auto finished_future = finished.get_future();
  finished_future.wait();
  client.stop();

  finished_future.get();

  std::cout << std::endl;

  return result;
}

nlohmann::json FindTurnData(const std::vector<nlohmann::json>& all_turns,
                            int turn) {
  for (const auto& json : all_turns) {
    if (json["Turn"] == turn) {
      return json;
    }
  }

  throw DownloaderException("Turn not found: " + std::to_string(turn));
}

nlohmann::json ConvertToSnakeData(const nlohmann::json& turn_data,
                                  const std::string& snake_id) {
  std::cout << "Snakes:" << std::endl;
  for (const auto& snake : turn_data["Snakes"]) {
    std::cout << "  " << std::string(snake["ID"]) << "  "
              << std::string(snake["Name"]) << std::endl;
  }

  if (snake_id.empty()) {
    throw DownloaderException("Please provide part of snake ID or name");
  }

  bool snake_found = false;
  nlohmann::json snake_data;
  for (const auto& snake : turn_data["Snakes"]) {
    std::string id = snake["ID"];
    std::string name = snake["Name"];
    if (id.find(snake_id) != std::string::npos ||
        name.find(snake_id) != std::string::npos) {
      std::cout << "Found snake: " << id << "  " << name << std::endl;
      if (snake_found) {
        throw DownloaderException("More than one snake found: " + snake_id);
      }
      snake_found = true;
      snake_data = snake;
    }
  }

  if (!snake_found) {
    throw DownloaderException("Snake not found: " + snake_id);
  }

  throw DownloaderException("Not implemented");
}

int main(int argc, const char* const argv[]) {
  try {
    DownloaderOptions options = ParseDownloaderOptions(argc, argv);
    if (options.exit_immediately) {
      return options.ret_code;
    }

    std::cout << options;

    auto turns_data = DownloadGameData(options);
    std::cout << "Got " << turns_data.size() << " turns" << std::endl;

    nlohmann::json turn_data = FindTurnData(turns_data, options.turn);
    nlohmann::json snake_data = ConvertToSnakeData(turn_data, options.snake);

    return 0;
  } catch (DownloaderException e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
