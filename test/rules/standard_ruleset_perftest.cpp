#include <battlesnake/rules/standard_ruleset.h>

#include <iostream>
#include <typeinfo>

using namespace ::battlesnake::rules;

template <typename T>
void TestType() {
  const std::type_info& info = typeid(T);
  std::cout << info.name() << ": " << std::is_pod<T>::value << "  "
            << std::is_trivially_destructible<T>::value << std::endl;
}

void TestTypes() {
  struct Point2 {
    int x;
    int y;

    bool operator==(const Point2& other) const {
      return this->x == other.x && this->y == other.y;
    }
    bool operator!=(const Point2& other) const { return !operator==(other); }

    Point2 Up() const { return Point2{x, y + 1}; }
    Point2 Down() const { return Point2{x, y - 1}; }
    Point2 Left() const { return Point2{x - 1, y}; }
    Point2 Right() const { return Point2{x + 1, y}; }
  };

  TestType<Point2>();
  TestType<Point>();
  TestType<Snake>();
  TestType<BoardState>();

  //   std::cout << "A: " << std::is_pod<P>::value << std::endl;
  //   std::cout << "Point: " << std::is_pod<Point>::value << std::endl;
  //   std::cout << "Snake: " << std::is_pod<Snake>::value << std::endl;
  //   std::cout << "BoardState: " << std::is_pod<BoardState>::value <<
  //   std::endl;
}

int main() {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeMedium,
      .height = kBoardSizeMedium,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("three"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
              Snake{
                  .id = pool.Add("four"),
                  .body =
                      {
                          Point{1, 1},
                          Point{1, 2},
                          Point{1, 3},
                      },
                  .health = 100,
              },
          },
  };

  const std::unordered_map<SnakeId, Move>& moves{
      {pool.Add("one"), Move::Up},
      {pool.Add("two"), Move::Down},
      {pool.Add("three"), Move::Left},
      {pool.Add("four"), Move::Right},
  };

  StandardRuleset ruleset{StandardRuleset::Config{
      .food_spawn_chance = 0,
      .minimum_food = 0,
      .optimize_strings = true,
  }};

  constexpr int max_size = 100000;
  std::vector<BoardState> states;
  states.reserve(max_size);

  for (int i = 0; i < max_size; ++i) {
    // BoardState state = ruleset.CreateNextBoardState(initial_state, moves);

    // BoardState new_state = initial_state;
    //  new_state.food.push_back(Point{0, 0});

    states.push_back(initial_state);
  }

  TestTypes();

  return 0;
}
