#include <battlesnake/interface/battlesnake.h>

#include <functional>
#include <memory>
#include <thread>

namespace battlesnake {
namespace server {

class BattlesnakeServer {
 public:
  BattlesnakeServer(battlesnake::interface::Battlesnake* battlesnake, int port,
                    int threads = 32);
  ~BattlesnakeServer();

  void Run(
      const std::function<void(unsigned short /*port*/)>& callback = nullptr);
  void Stop();

  // Convenience function that runs the server on a new thread and returns when
  // the server is ready to accept connections. Returns thread handle.
  std::unique_ptr<std::thread> RunOnNewThread();

 private:
  class BattlesnakeServerImpl;

  std::unique_ptr<BattlesnakeServerImpl> impl;
};

}  // namespace server
}  // namespace battlesnake
