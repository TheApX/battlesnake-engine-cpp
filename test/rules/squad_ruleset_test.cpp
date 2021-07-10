#include "battlesnake/rules/squad_ruleset.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace battlesnake {
namespace rules {

namespace {

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Field;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Ne;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

template <class M>
auto SnakeIdIs(M m) {
  return Field(&Snake::id, m);
}

template <class M>
auto SnakeIsEliminated(M m) {
  return AllOf(SnakeIdIs(m), Field(&Snake::eliminated_cause,
                                   Field(&EliminatedCause::cause,
                                         Ne(EliminatedCause::NotEliminated))));
}

template <class M>
auto SnakeIsNotEliminated(M m) {
  return AllOf(SnakeIdIs(m), Field(&Snake::eliminated_cause,
                                   Field(&EliminatedCause::cause,
                                         Eq(EliminatedCause::NotEliminated))));
}

template <class A, class B, class C, class D, class E>
auto SnakeIs(const A& m_id, const B& m_body, const C& m_health,
             const D& m_cause, const E& m_eliminated_by) {
  return AllOf(
      Field(&Snake::id, m_id), Field(&Snake::body, m_body),
      Field(&Snake::health, m_health),
      Field(&Snake::eliminated_cause, Field(&EliminatedCause::cause, m_cause)),
      Field(&Snake::eliminated_cause,
            Field(&EliminatedCause::by_id, m_eliminated_by)));
}

template <class A, class B, class C, class D>
auto SnakeIs(const A& m_id, const B& m_body, const C& m_health,
             const D& m_cause) {
  return SnakeIs(m_id, m_body, m_health, m_cause, _);
}

template <class A, class B, class C>
auto SnakeIs(const A& m_id, const B& m_body, const C& m_health) {
  return SnakeIs(m_id, m_body, m_health, _, _);
}

template <class A, class B>
auto SnakeIs(const A& m_id, const B& m_body) {
  return SnakeIs(m_id, m_body, _, _, _);
}

class SquadRulesetTest : public testing::Test {
 protected:
  std::vector<SnakeId> CreateSnakeIds(int n, StringPool& pool) {
    std::vector<SnakeId> result;
    result.reserve(n);
    for (int i = 0; i < n; ++i) {
      result.push_back(pool.Add("Snake" + std::to_string(n)));
    }
    return result;
  }
};

TEST_F(SquadRulesetTest, Sanity) {
  SquadRuleset ruleset;

  BoardState state = ruleset.CreateInitialBoardState(0, 0, {});
  EXPECT_THAT(state.width, Eq(0));
  EXPECT_THAT(state.height, Eq(0));
  EXPECT_THAT(state.snakes, ElementsAre());

  BoardState new_state = ruleset.CreateNextBoardState(state, {}, 0);
  EXPECT_THAT(state.width, Eq(0));
  EXPECT_THAT(state.height, Eq(0));
  EXPECT_THAT(state.snakes, ElementsAre());

  EXPECT_THAT(ruleset.IsGameOver(state), IsTrue());
}

class SquadCreateNextBoardStateTest : public SquadRulesetTest {};

TEST_F(SquadCreateNextBoardStateTest, SameSquadDontCollide) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                      },
                  .health = 100,
                  .squad = pool.Add("s"),
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 1},
                          Point{2, 1},
                          Point{3, 1},
                      },
                  .health = 100,
                  .squad = pool.Add("s"),
              },
          },
  };

  SquadRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   1);

  EXPECT_THAT(state.snakes, UnorderedElementsAre(SnakeIsNotEliminated("one"),
                                                 SnakeIsNotEliminated("two")));
}

TEST_F(SquadCreateNextBoardStateTest, DifferentSquadCollide) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                      },
                  .health = 100,
                  .squad = pool.Add("s1"),
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 1},
                          Point{2, 1},
                          Point{3, 1},
                      },
                  .health = 100,
                  .squad = pool.Add("s2"),
              },
          },
  };

  SquadRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   1);

  EXPECT_THAT(state.snakes, UnorderedElementsAre(SnakeIsNotEliminated("one"),
                                                 SnakeIsEliminated("two")));
}

TEST_F(SquadCreateNextBoardStateTest, ShareHealth) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                      },
                  .health = 10,
                  .squad = pool.Add("s"),
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 1},
                          Point{2, 1},
                          Point{3, 1},
                      },
                  .health = 100,
                  .squad = pool.Add("s"),
              },
          },
  };

  SquadRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   1);

  EXPECT_THAT(state.snakes, UnorderedElementsAre(SnakeIs("one", _, 99),
                                                 SnakeIs("two", _, 99)));
}

TEST_F(SquadCreateNextBoardStateTest, ShareLength) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                          Point{0, 4},
                          Point{0, 5},
                      },
                  .health = 10,
                  .squad = pool.Add("s"),
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 1},
                          Point{2, 1},
                          Point{3, 1},
                      },
                  .health = 100,
                  .squad = pool.Add("s"),
              },
          },
  };

  SquadRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   1);

  EXPECT_THAT(state.snakes, UnorderedElementsAre(SnakeIs("one", SizeIs(5)),
                                                 SnakeIs("two", SizeIs(5))));
}

TEST_F(SquadCreateNextBoardStateTest, ShareElimination) {
  StringPool pool;
  BoardState initial_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                          Point{0, 4},
                          Point{0, 5},
                      },
                  .health = 10,
                  .squad = pool.Add("s"),
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{0, 1},
                          Point{1, 1},
                          Point{2, 1},
                          Point{3, 1},
                      },
                  .health = 100,
                  .squad = pool.Add("s"),
              },
          },
  };

  SquadRuleset ruleset(StandardRuleset::Config{.food_spawn_chance = 0});
  BoardState state =
      ruleset.CreateNextBoardState(initial_state,
                                   {
                                       {pool.Add("one"), Move::Down},
                                       {pool.Add("two"), Move::Left},
                                   },
                                   1);

  EXPECT_THAT(
      state.snakes,
      UnorderedElementsAre(SnakeIs("one", _, _, EliminatedCause::BySquad),
                           SnakeIs("two", _, _, EliminatedCause::OutOfBounds)));
}

class SquadIsGameOverTest : public SquadRulesetTest {};

TEST_F(SquadIsGameOverTest, ZeroSnakes) {
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes = {},
  };

  SquadRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

TEST_F(SquadIsGameOverTest, TwoSnakesSameSquad) {
  StringPool pool;
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                      },
                  .health = 100,
                  .squad = pool.Add("s"),
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 1},
                          Point{2, 1},
                          Point{3, 1},
                      },
                  .health = 100,
                  .squad = pool.Add("s"),
              },
          },
  };

  SquadRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsTrue());
}

TEST_F(SquadIsGameOverTest, TwoSnakesDifferentSquad) {
  StringPool pool;
  BoardState board_state{
      .width = kBoardSizeSmall,
      .height = kBoardSizeSmall,
      .snakes =
          {
              Snake{
                  .id = pool.Add("one"),
                  .body =
                      {
                          Point{0, 1},
                          Point{0, 2},
                          Point{0, 3},
                      },
                  .health = 100,
                  .squad = pool.Add("s1"),
              },
              Snake{
                  .id = pool.Add("two"),
                  .body =
                      {
                          Point{1, 1},
                          Point{2, 1},
                          Point{3, 1},
                      },
                  .health = 100,
                  .squad = pool.Add("s2"),
              },
          },
  };

  SquadRuleset ruleset;

  EXPECT_THAT(ruleset.IsGameOver(board_state), IsFalse());
}

}  // namespace

}  // namespace rules
}  // namespace battlesnake
