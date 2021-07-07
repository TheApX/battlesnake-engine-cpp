#include "options.h"

#include <argparse/argparse.hpp>
#include <regex>
#include <string>

std::ostream& operator<<(std::ostream& str, const DownloaderOptions& options) {
  if (options.exit_immediately || options.ret_code != 0) {
    str << "Return code: " << options.ret_code;
    if (!options.exit_immediately) {
      str << " (not forced)";
    }
    str << std::endl;
  }

  str << "Game ID:   " << options.game_id << std::endl;
  str << "Turn:      " << options.turn << std::endl;
  str << "Snake:     " << options.snake << std::endl;
  str << "Filename:  " << options.filename << std::endl;

  return str;
}

DownloaderOptions ParseDownloaderOptions(int argc, const char* const argv[]) {
  DownloaderOptions result;

  argparse::ArgumentParser arguments("BattleSnake CLI");

  arguments["-h"].default_value(false).implicit_value(true);

  arguments.add_argument("-g", "--gameid")
      .help("game ID or url")
      .default_value(result.game_id);
  arguments.add_argument("-t", "--turn")
      .help("turn number")
      .action([](const std::string& value) { return std::stoi(value); })
      .default_value(result.turn);
  arguments.add_argument("-s", "--snake")
      .help("part of snake name or ID")
      .default_value(result.snake);
  arguments.add_argument("-f", "--filename")
      .help("path to output json file")
      .default_value(result.filename);

  try {
    arguments.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << arguments;
    return DownloaderOptions{.exit_immediately = true, .ret_code = 1};
  }

  if (arguments.get<bool>("-h")) {
    std::cout << arguments;
    return DownloaderOptions{.exit_immediately = true, .ret_code = 0};
  }

  result.game_id = arguments.get<std::string>("--gameid");
  result.turn = arguments.get<int>("--turn");
  result.snake = arguments.get<std::string>("--snake");
  result.filename = arguments.get<std::string>("--filename");

  if (result.game_id.empty()) {
    std::cout << "No game ID provided" << std::endl;
    std::cout << arguments;
    return DownloaderOptions{.exit_immediately = true, .ret_code = 2};
  }

  std::regex game_url_regex(
      R"regex(https://play.battlesnake.com/g/([0-9a-fA-F\-]+)/)regex");
  std::smatch game_url_match;
  if (std::regex_match(result.game_id, game_url_match, game_url_regex)) {
    if (game_url_match.size() != 2) {
      std::cout << "Invalid game URL: " << result.game_id << std::endl;
      return DownloaderOptions{.exit_immediately = true, .ret_code = 3};
    }
    result.game_id = game_url_match[1];
  }

  if (result.filename == kDefaultFilename) {
    result.filename =
        result.game_id + "-turn-" + std::to_string(result.turn) + ".json";
  }

  return result;
}
