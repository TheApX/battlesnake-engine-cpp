#include <battlesnake/server/server.h>

#include <iostream>

#include "snake_random.h"

int main() {
  SnakeRandom snake;
  battlesnake::server::BattlesnakeServer server(&snake, 12388);

  server.Run([](unsigned short port) {
    std::cout << "Snake is running on port " << port << std::endl;
  });

  return 0;
}
