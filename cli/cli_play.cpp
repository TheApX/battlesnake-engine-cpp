#include <curl/curl.h>

#include <memory>

#include "battlesnake/rules/ruleset.h"
#include "battlesnake/rules/standard_ruleset.h"
#include "cli_options.h"

namespace battlesnake {
namespace cli {

namespace {

using namespace battlesnake::rules;

class CurlInit {
 public:
  CurlInit() { curl_global_init(CURL_GLOBAL_ALL); }
  ~CurlInit() { curl_global_cleanup(); }
};

}  // namespace

std::unique_ptr<Ruleset> CreateRuleset(const std::string& name) {
  if (name == "standard") {
    return std::make_unique<StandardRuleset>();
  }

  return nullptr;
}

int PlayGame(const CliOptions& options) {
  CurlInit curl_init;

  std::unique_ptr<Ruleset> ruleset = CreateRuleset(options.gametype);
  if (ruleset == nullptr) {
    std::cerr << "Unknown game type: " << options.gametype << std::endl;
    return 10;
  }

  return 0;
}

}  // namespace cli
}  // namespace battlesnake
