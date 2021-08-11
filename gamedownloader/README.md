# BattleSnake game downloader

BattleSnake game downloader downloads game json for a specific turn and creates json file that can be provided directly to your battlesnake's `/move` endpoint.

# Build

Install the following packages:

```
sudo apt install \
    build-essential \
    cmake \
    clang \
    libcurl4-openssl-dev \
    libssl-dev
```

Build `battlesnakedownloader` target using CMake. You can also use convenience script that to build everything and run tests:

```
./build.sh
```

# Run

Download using game URL, turn number and any unique part of snake name:

```
./build/gamedownloader/battlesnakedownloader \
    -g https://play.battlesnake.com/g/50c90f02-a22d-4706-b6bb-dc42c27f0565/ \
    --turn 153 \
    --snake Oracle
```

You can also provide output file name. Game URL can be shortened to just game ID. For example:

```
./build/gamedownloader/battlesnakedownloader \
    -g 50c90f02-a22d-4706-b6bb-dc42c27f0565 \
    --turn 153 \
    --snake Oracle \
    --filename my-game.json
```

# Test your snake

You can send downloaded json file to your snake using `curl`:

```
curl \
    -X POST \
    -H "Content-Type: application/json" \
    -d @my-game.json \
    http://your-snake-host:port/move
```
