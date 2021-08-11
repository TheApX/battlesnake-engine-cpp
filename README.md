# C++ implementation of BattleSnake rules and snake server

C++ implementation of https://github.com/BattlesnakeOfficial/rules - official battlesnake.com rules. Plus some extra useful tools.

# What's included

* C++ structs definitions for json passed to battlesnakes.
  * Converters to/from json.
* Base abstraction for different game modes (rules).
* Game modes implemented:
  * standard
  * solo
  * royale
  * squad
  * constrictor
* CLI tool for running games.
  * Demonstrates how to use game rules.
* Web-server for running battlesnakes.
  * All you need to implement is a simple API with 4 methods - one for each type of request.
  * json conversions are done by server.
* Simple random battlesnake.
  * Demonstrates how to use web-server and build your battlesnakes.
  * Fast unit tests that don't use web-server.
* Battlesnake [game downloader](gamedownloader/README.md).

# Building and running

Install the following packages:

```
sudo apt install \
    build-essential \
    cmake \
    clang \
    libcurl4-openssl-dev \
    libssl-dev
```

Build using CMake. You can also use convenience script that to build everything and run tests:

```
./build.sh
```

Start battlesnake

```
./build/snakes/random/battlesnake_random
```

## Platforms supported

The only tested platform is Linux. CPU architectures tested are:
* x86_64
* AArch64 (arm64)
* armv7l

The battlesnake engine itself is written vanilla C++17 and all libraries are cross-platform, so there should be no problems running it on Windows or Mac (including both Intel and ARM).

The only potential problem is that CLI uses some unicode characters to render snakes, which may not be supported on Windows. It definitely works fine in Windows Terminal + WSL2 + Ubuntu 20.04 though.

If any adjustments needed to build and run on other platforms, please send a pull request. Contributions are welcome!

# Q&A

* Why?
  * Because I need Battlesnake rules implemented in C++ for my future snakes.
* Why C++?
  * It is fast. During the same 500ms code written in vanilla C++ can compute much more than vanilla Python, for example.
  * It's the language I've been using at my main job for over 10 years.
* Can I use it as a starter project?
  * Yes, but I wouldn't recommend. It is a bit overly complicated for a starter project. Though there is an [example](snakes/random/README.md) that you can use.
* I want to make contribution!
  * You are more than welcome! Please send a pull request and I will respond in a couple days.

# Credits

These libraries are pulled from their official repositories and built from source during build:

* https://github.com/google/googletest
* https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
* https://github.com/p-ranav/argparse
* https://github.com/mariusbancila/stduuid
* https://github.com/kingsamchen/asio-cmake
  * https://github.com/chriskohlhoff/asio
* https://gitlab.com/eidheim/Simple-Web-Server
* https://github.com/eidheim/Simple-WebSocket-Server
* https://github.com/iboB/itlib
