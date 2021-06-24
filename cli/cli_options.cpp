#include "cli_options.h"

namespace battlesnake {
namespace cli {

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

}  // namespace cli
}  // namespace battlesnake
