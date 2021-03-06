#pragma once

#include <unordered_map>

#include "data_types.h"

namespace battlesnake {
namespace rules {

std::string RenderGame(
    const GameState& state,
    const std::unordered_map<SnakeId, char>& snake_head_syms);

}  // namespace rules
}  // namespace battlesnake
