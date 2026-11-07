#pragma once

#include <cstdint>

namespace neomaxis {

struct GameState {
    std::uint32_t elapsed_days = 0;
    std::uint16_t start_year = 0;
};

bool TryReadGameState(GameState& state);

}
