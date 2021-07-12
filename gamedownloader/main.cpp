#include <curl/curl.h>

#include <client_wss.hpp>
#include <exception>
#include <fstream>
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

class CurlInit {
 public:
  CurlInit() { curl_global_init(CURL_GLOBAL_ALL); }
  ~CurlInit() { curl_global_cleanup(); }
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                            void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

std::string HttpRequest(const std::string& url, int timeout_ms,
                        const std::string& method = "GET",
                        const std::string& data = "") {
  CURL* curl = curl_easy_init();
  if (curl == nullptr) {
    throw std::runtime_error("Can't initialize curl");
  }

  std::string readBuffer;

  struct curl_slist* headers = NULL;
  headers = curl_slist_append(headers, "Expect:");
  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "charset: utf-8");

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  if (method != "GET") {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
  }
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "battlesnakecpp-cli/0.1");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK) {
    std::cerr << "res = " << res << std::endl;
    // abort();
  }

  return readBuffer;
}

nlohmann::json DownloadGameInfo(const std::string& game_id) {
  return nlohmann::json::parse(
      HttpRequest("https://engine.battlesnake.com/games/" + game_id, 60000));
}

std::vector<nlohmann::json> DownloadGameData(const std::string& game_id) {
  std::vector<nlohmann::json> result;

  WssClient client("engine.battlesnake.com/socket/" + game_id,
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

nlohmann::json ConvertGameInfo(const nlohmann::json& info) {
  return nlohmann::json{
      {"id", info["ID"]},
      {"timeout", info["SnakeTimeout"]},
      {"ruleset",
       {
           {"name", info["Ruleset"]["name"]},
           {"version", "v1.0.0"},  // Version is not available, use fake.
       }},
  };
}

nlohmann::json ConvertPoint(const nlohmann::json& p) {
  return nlohmann::json{
      {"x", p["X"]},
      {"y", p["Y"]},
  };
}

nlohmann::json ConvertPointArray(const nlohmann::json& info) {
  nlohmann::json result = nlohmann::json::array();

  for (const nlohmann::json& p : info) {
    result.push_back(ConvertPoint(p));
  }

  return result;
}

bool IsSnakeAlive(const nlohmann::json& info) {
  auto death = info.find("Death");
  if (death == info.end()) {
    return true;
  }

  return death->is_null();
}

nlohmann::json ConvertSnake(const nlohmann::json& info) {
  return nlohmann::json{
      {"body", ConvertPointArray(info["Body"])},
      {"head", ConvertPoint(info["Body"].front())},
      {"health", info["Health"]},
      {"id", info["ID"]},
      {"latency", info["Latency"]},
      {"length", info["Body"].size()},
      {"name", info["Name"]},
      {"shout", info["Shout"]},
      {"squad", info["Squad"]},
  };
}

nlohmann::json ConvertSnakesArray(const nlohmann::json& info) {
  nlohmann::json result = nlohmann::json::array();

  for (const nlohmann::json& s : info) {
    if (!IsSnakeAlive(s)) {
      continue;
    }
    result.push_back(ConvertSnake(s));
  }

  return result;
}

nlohmann::json ConvertBoard(const nlohmann::json& info,
                            const nlohmann::json& game_info) {
  return nlohmann::json{
      {"food", ConvertPointArray(info["Food"])},
      {"hazards", ConvertPointArray(info["Hazards"])},
      {"width", game_info["Width"]},
      {"height", game_info["Height"]},
      {"snakes", ConvertSnakesArray(info["Snakes"])},
  };
}

nlohmann::json ConvertToSnakeData(const nlohmann::json& game_info,
                                  const nlohmann::json& turn_data,
                                  const nlohmann::json& snake_data) {
  return nlohmann::json{
      {"turn", turn_data["Turn"]},
      {"game", ConvertGameInfo(game_info["Game"])},
      {"you", ConvertSnake(snake_data)},
      {"board", ConvertBoard(turn_data, game_info["Game"])},
  };
}

nlohmann::json ConvertToSnakeData(const nlohmann::json& game_info,
                                  const nlohmann::json& turn_data,
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

  return ConvertToSnakeData(game_info, turn_data, snake_data);
}

}  // namespace

int main(int argc, const char* const argv[]) {
  try {
    CurlInit curl_init;

    DownloaderOptions options = ParseDownloaderOptions(argc, argv);
    if (options.exit_immediately) {
      return options.ret_code;
    }

    std::cout << options;

    auto game_info = DownloadGameInfo(options.game_id);
    auto turns_data = DownloadGameData(options.game_id);
    std::cout << "Got " << turns_data.size() << " turns" << std::endl;

    nlohmann::json turn_data = FindTurnData(turns_data, options.turn);
    nlohmann::json snake_data =
        ConvertToSnakeData(game_info, turn_data, options.snake);

    {
      std::ofstream out(options.filename);
      if (!out.good()) {
        throw DownloaderException("Can't open file: " + options.filename);
      }
      out << snake_data.dump(4);
      std::cout << "Saved game state to: " << options.filename << std::endl;
    }

    return 0;
  } catch (DownloaderException e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}
