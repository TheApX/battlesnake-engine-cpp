#include <argparse/argparse.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "uuid.h"

struct SnakeNameUrl {
  std::string name;
  std::string url;
};

struct CliOptions {
  bool exit_immediately = false;
  int ret_code = 0;

  int width = 11;
  int height = 11;
  std::vector<SnakeNameUrl> snakes;
  bool view_map = false;
};

std::ostream& operator<<(std::ostream& str, const CliOptions& options) {
  if (options.exit_immediately || options.ret_code != 0) {
    str << "Return code: " << options.ret_code;
    if (!options.exit_immediately) {
      str << " (not forced)";
    }
    str << std::endl;
  }

  str << "Size:          " << options.width << "x" << options.height
      << std::endl;
  str << "View map:      " << (options.view_map ? "true" : "false")
      << std::endl;
  str << "Snakes:" << std::endl;
  for (const SnakeNameUrl& snake_info : options.snakes) {
    str << "  " << snake_info.name << "    " << snake_info.url << std::endl;
  }

  return str;
}

std::string GenerateName() {
  std::random_device rd;
  auto seed_data = std::array<int, std::mt19937::state_size>{};
  std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
  std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
  std::mt19937 generator(seq);
  uuids::uuid_random_generator gen{generator};

  uuids::uuid id = gen();
  return uuids::to_string(id);
}

CliOptions ParseOptions(int argc, const char* const argv[]) {
  CliOptions result;

  argparse::ArgumentParser arguments("BattleSnake CLI");

  arguments["-h"].default_value(false).implicit_value(true);

  arguments.add_argument("-W", "--width")
      .help("width of board")
      .action([](const std::string& value) { return std::stoi(value); })
      .default_value(result.width);
  arguments.add_argument("-H", "--height")
      .help("height of board")
      .action([](const std::string& value) { return std::stoi(value); })
      .default_value(result.height);
  arguments.add_argument("-n", "--name")
      .help("name of snake")
      .default_value(std::vector<std::string>{})
      .append();
  arguments.add_argument("-u", "--url")
      .help("URL of snake")
      .default_value(std::vector<std::string>{})
      .append();
  arguments.add_argument("-v", "--viewmap")
      .help("view map of each turn")
      .default_value(false);

  try {
    arguments.parse_args(argc, argv);
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << arguments;
    return CliOptions{.exit_immediately = true, .ret_code = 1};
  }

  if (arguments.get<bool>("-h")) {
    std::cout << arguments;
    return CliOptions{.exit_immediately = true, .ret_code = 0};
  }

  result.width = arguments.get<int>("-W");
  result.height = arguments.get<int>("-H");
  result.view_map = arguments.get<bool>("-v");

  std::vector<std::string> names =
      arguments.get<std::vector<std::string>>("-n");
  std::vector<std::string> urls = arguments.get<std::vector<std::string>>("-u");

  for (int i = 0; i < std::max(names.size(), urls.size()); ++i) {
    if (i >= urls.size()) {
      std::cerr << "No URL provided for snake '" << names[i] << "'"
                << std::endl;
      result.exit_immediately = true;
      result.ret_code = 2;
      continue;
    }

    result.snakes.push_back(SnakeNameUrl{
        .name = i < names.size() ? names[i] : GenerateName(),
        .url = urls[i],
    });
  }

  if (result.snakes.empty()) {
    std::cout << "No snake URLs provided" << std::endl;
    std::cout << arguments;
    return CliOptions{.exit_immediately = true, .ret_code = 3};
  }

  return result;
}

int main(int argc, const char* const argv[]) {
  CliOptions options = ParseOptions(argc, argv);
  if (options.exit_immediately) {
    return options.ret_code;
  }

  std::cout << options;

  return 0;
}
