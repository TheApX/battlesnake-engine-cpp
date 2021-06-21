#include "battlesnake/json/converter.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace json {

namespace {

using ::testing::Eq;

using namespace ::battlesnake::engine;

class ConverterTest : public testing::Test {};

class PointTest : public ConverterTest {};

TEST_F(PointTest, CreateJson) {
  nlohmann::json json = CreateJson(Point{.x = 123, .y = 456});
  EXPECT_THAT(json, Eq(nlohmann::json::parse(R"json({"x":123,"y":456})json")));
}

}  // namespace

}  // namespace json
}  // namespace battlesnake
