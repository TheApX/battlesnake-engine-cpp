#include "battlesnake/interface/battlesnake.h"

namespace battlesnake {
namespace interface {

void Battlesnake::GetCustomization(
    std::function<void(const battlesnake::rules::Customization& result)>
        respond) {
  respond(GetCustomization());
};

void Battlesnake::Start(
    std::shared_ptr<battlesnake::rules::StringPool> string_pool,
    const battlesnake::rules::GameState& game_state,
    std::function<void()> respond) {
  Start(game_state);
  respond();
};

void Battlesnake::End(
    std::shared_ptr<battlesnake::rules::StringPool> string_pool,
    const battlesnake::rules::GameState& game_state,
    std::function<void()> respond) {
  End(game_state);
  respond();
};

void Battlesnake::Move(
    std::shared_ptr<battlesnake::rules::StringPool> string_pool,
    const battlesnake::rules::GameState& game_state,
    std::function<void(const MoveResponse& result)> respond) {
  respond(Move(game_state));
};

}  // namespace interface
}  // namespace battlesnake
