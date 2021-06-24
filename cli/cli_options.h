#include <iostream>
#include <string>
#include <vector>

namespace battlesnake {
namespace cli {

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

std::ostream& operator<<(std::ostream& str, const CliOptions& options);

}  // namespace cli
}  // namespace battlesnake
