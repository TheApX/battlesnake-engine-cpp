# Random BattleSnake

This is an example that shows how to implement a BattleSnake using the web-server and interfaces provided by battlesnake-engine-cpp. It includes unit tests that don't run the whole web-server and test only the core BattleSnake logic.

# Project structure

* [main.cpp](main.cpp)
  * `main` function that initializes the snake and runs the server.
* [snake_random.h](snake_random.h)
  * Snake class definition.
* [snake_random.cpp](snake_random.cpp)
  * Main snake logic. You need to update it to make the snake smarter.
* [snake_random_test.cpp](snake_random_test.cpp)
  * Unit tests for the snake. Write your tests there.
* [testdata](testdata)
  * Directory with json files with game state used for tests.
* [CMakeLists.txt](CMakeLists.txt)
  * cmake configuration file for the battlesnake executable and tests.

# Building, running and testing

To build and run battlesnake run this command from the root directory of the repo:

```
./run_snake_random.sh
```

After the snake is build, you can run it manually:

```
./build/snakes/random/battlesnake_random
```

This executable is the only file needed to run the battlesnake. You can copy it to your server and run there.

To build and run unit tests, run from the root directory:

```
./test_snake_random.sh
```

# Snake implementation

## GetCustomization

Returns a `Customization` object that contains information about snake's color, head, tail, author, etc. This snake sets only color, head and tail. See [struct definition](../../include/battlesnake/rules/data_types.h) for details.

## Start

Called when a new game starts. This snakes does nothing.

## End

Called when a new game ends. This snakes does nothing.

## Move

Called for each move. This snakes fills in a vector of possible moves and selects a random element from this vector.

# Unit tests

Unit tests are implemented in [snake_random_test.cpp](snake_random_test.cpp).

There is one test for `GetCustomization()`, which tests that snake is not a boring grey one with default head and tail.

The most important part of the snake is `Move()` method. It takes a `GameState` object and returns the direction where it wants to move. There are two ways to construct `GameState` for a test:
* Construct manually, like it is done in the `SnakeMoves` test.
* Load from json file, like it is done in the `LoadTestFromJson` test.

Test json must contain exactly the same json object as the battlesnake engine sends. It is loaded and converted to `GameState` object in the `LoadState` function, defined at the beginning of the test file. Note that this function takes just base name of the json file. If you provide `LoadState("test001")` loads file `test001.json` from the `testdata` directory.
