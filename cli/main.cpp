#include <iostream>
#include <string>

#include "cli_options.h"
#include "cli_play.h"

using namespace battlesnake::cli;

int main(int argc, const char* const argv[]) {
  CliOptions options = ParseOptions(argc, argv);
  if (options.exit_immediately) {
    return options.ret_code;
  }

  std::cout << options;

  return PlayGame(options);
}
