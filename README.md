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

# Building and running

Install the following packages:

```
sudo apt install \
    build-essential \
    cmake \
    clang \
    libcurl4-openssl-dev
```

Build everything and run tests:

```
./build.sh
```

Start battlesnake

```
./build/snakes/random/battlesnake_random
```

# Credits

These libraries are pulled from their official repositories and built from source during build:

* https://github.com/google/googletest
* https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
* https://github.com/p-ranav/argparse
* https://github.com/mariusbancila/stduuid
* https://github.com/kingsamchen/asio-cmake
  * https://github.com/chriskohlhoff/asio
* https://gitlab.com/eidheim/Simple-Web-Server
