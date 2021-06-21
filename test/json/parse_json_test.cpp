#include "battlesnake/json/converter.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace json {

namespace {

using ::testing::Eq;
using ::testing::IsNull;
using ::testing::Pointee;

using namespace ::battlesnake::engine;

class ParseJsonTest : public testing::Test {};

TEST_F(ParseJsonTest, PointSucceeds) {
  EXPECT_THAT(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"y":456})json")),
      Eq(Point(123, 456)));
}

TEST_F(ParseJsonTest, PointFails) {
  EXPECT_THROW(
      ParseJsonPoint(nlohmann::json::parse(R"json({"x":123,"z":456})json")),
      ParseException);
}

}  // namespace

}  // namespace json
}  // namespace battlesnake
