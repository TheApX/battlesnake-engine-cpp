#include <iostream>
#include <string>

#include "options.h"

int main(int argc, const char* const argv[]) {
  DownloaderOptions options = ParseDownloaderOptions(argc, argv);
  if (options.exit_immediately) {
    return options.ret_code;
  }

  std::cout << options;

  return -1;
}
