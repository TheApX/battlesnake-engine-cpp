#include "http_client_battlesnake.h"

#include <battlesnake/json/converter.h>
#include <curl/curl.h>

#include <nlohmann/json.hpp>

namespace battlesnake {
namespace cli {

namespace {

using namespace battlesnake::rules;

std::string SanitizeUrl(const std::string& url) {
  if (!url.empty() && url[url.size() - 1] != '/') {
    return url + "/";
  }
  return url;
}

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
  headers = curl_slist_append(headers, "Accept: application/json");
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "charset: utf-8");

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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

  return readBuffer;
}

}  // namespace

HttpClientBattlesnake::HttpClientBattlesnake(const std::string& url)
    : url_(SanitizeUrl(url)) {}

HttpClientBattlesnake::~HttpClientBattlesnake() {}

Customization HttpClientBattlesnake::GetCustomization() {
  try {
    std::string response = HttpRequest(url_, 500);
    return battlesnake::json::ParseJsonCustomization(
        nlohmann::json::parse(response));
  } catch (std::exception) {
    return Customization{};
  }
}

void HttpClientBattlesnake::Start(const GameState& game_state) {
  nlohmann::json game_json = battlesnake::json::CreateJson(game_state);
  std::string game_json_str = game_json.dump();
  HttpRequest(url_ + "start", game_state.game.timeout, "POST", game_json_str);
}

void HttpClientBattlesnake::End(const GameState& game_state) {
  nlohmann::json game_json = battlesnake::json::CreateJson(game_state);
  std::string game_json_str = game_json.dump();
  HttpRequest(url_ + "end", game_state.game.timeout, "POST", game_json_str);
}

HttpClientBattlesnake::MoveResponse HttpClientBattlesnake::Move(
    const GameState& game_state) {
  nlohmann::json game_json = battlesnake::json::CreateJson(game_state);
  std::string game_json_str = game_json.dump();

  std::string response = HttpRequest(url_ + "move", game_state.game.timeout,
                                     "POST", game_json_str);

  try {
    nlohmann::json r = nlohmann::json::parse(response);
    if (!r.is_object()) {
      return MoveResponse();
    }

    std::string move = r["move"];

    MoveResponse response;

    if (move == "up") response.move = Move::Up;
    if (move == "down") response.move = Move::Down;
    if (move == "left") response.move = Move::Left;
    if (move == "right") response.move = Move::Right;

    auto shout = r.find("shout");
    if (shout != r.end()) {
      response.shout = *shout;
    }

    return response;
  } catch (std::exception) {
    return MoveResponse();
  }
  return MoveResponse();
}

}  // namespace cli
}  // namespace battlesnake
