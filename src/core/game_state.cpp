#include <cstdint>

#include "neomaxis/game_memory.h"
#include "neomaxis/game_state.h"

namespace neomaxis {

namespace {

constexpr std::uintptr_t CITY_START_YEAR_RVA = 0x000CA5F4;
constexpr std::uintptr_t CITY_DAYS_RVA       = 0x000CAE04;

}

bool TryReadGameState(GameState& state)
{
    GameState snapshot{};

    if (!TryReadGameMemory(
            CITY_START_YEAR_RVA,
            &snapshot.start_year,
            sizeof(snapshot.start_year)
        )) {
        return false;
    }

    if (!TryReadGameMemory(
            CITY_DAYS_RVA,
            &snapshot.elapsed_days,
            sizeof(snapshot.elapsed_days)
        )) {
        return false;
    }

    state = snapshot;

    return true;
}

}
