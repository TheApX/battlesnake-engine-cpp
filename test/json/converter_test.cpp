#include "battlesnake/json/converter.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace json {

namespace {

using ::testing::Eq;
using ::testing::IsNull;

using namespace ::battlesnake::engine;

class ConverterTest : public testing::Test {};

class PointTest : public ConverterTest {};

TEST_F(PointTest, CreateJson) {
  nlohmann::json json = CreateJson(Point(123, 456));
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({"x":123,"y":456})json")));
}

class SnakeTest : public ConverterTest {};

TEST_F(SnakeTest, EliminatedSnakeNullJson) {
  EXPECT_THAT(MayCreateJson(Snake{
                  .eliminated_cause =
                      EliminatedCause{.cause = EliminatedCause::OutOfHealth}}),
              IsNull());
}

TEST_F(SnakeTest, NotEliminatedSnakeJson) {
  Snake snake{
      .id = "snake_id",
      .body =
          {
              Point(10, 1),
              Point(10, 2),
              Point(10, 3),
          },
      .health = 75,
      .name = "Test Caterpillar",
      .latency = "123",
      .shout = "Why are we shouting???",
      .squad = "The Suicide Squad",
  };

  auto expected_json = nlohmann::json::parse(R"json(
      {
          "id": "snake_id",
          "body": [
              {"x": 10, "y": 1},
              {"x": 10, "y": 2},
              {"x": 10, "y": 3}
          ],
          "length": 3,
          "head": {"x": 10, "y": 1},
          "health": 75,
          "name": "Test Caterpillar",
          "latency": "123",
          "shout": "Why are we shouting???",
          "squad": "The Suicide Squad"
      }
  )json");

  EXPECT_THAT(MayCreateJson(snake), testing::Pointee(expected_json));
}

}  // namespace

}  // namespace json
}  // namespace battlesnake
