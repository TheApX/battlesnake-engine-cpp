#pragma once

#include <iostream>
#include <string>
#include <vector>

struct SnakeNameUrl {
  std::string name;
  std::string url;
};

static inline constexpr char const* kDefaultFilename =
    "<gameid>-turn-<turn>.json";

struct DownloaderOptions {
  bool exit_immediately = false;
  int ret_code = 0;

  std::string game_id = "";
  int turn = 0;
  std::string snake = "";

  std::string filename = kDefaultFilename;
};

std::ostream& operator<<(std::ostream& str, const DownloaderOptions& options);

DownloaderOptions ParseDownloaderOptions(int argc, const char* const argv[]);
