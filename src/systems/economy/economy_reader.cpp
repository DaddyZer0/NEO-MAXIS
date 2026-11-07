#include <cstdint>

#include "neomaxis/economy.h"
#include "neomaxis/economy_reader.h"
#include "neomaxis/game_memory.h"

namespace neomaxis {

namespace {

constexpr std::uintptr_t CITY_FUNDS_RVA = 0x000CA444;
constexpr std::uintptr_t CITY_BONDS_RVA = 0x000CC4E8;
constexpr std::uintptr_t CITY_LAND_VALUE_RVA = 0x000CA440;
constexpr std::uintptr_t CITY_VALUE_RVA = 0x000CA4D0;
constexpr std::uintptr_t INTEREST_RATE_SUM_RVA = 0x000CA400;

static_assert(sizeof(std::int32_t) == 4,
    "Economy memory fields require 32-bit integers.");

static_assert(sizeof(std::uint32_t) == 4,
    "Economy memory fields require 32-bit integers.");

}

bool TryReadEconomySnapshot(EconomySnapshot& snapshot)
{
    EconomySnapshot result{};

    if (!TryReadGameMemory(
            CITY_FUNDS_RVA,
            &result.city_funds,
            sizeof(result.city_funds)
        )) {
        return false;
    }

    if (!TryReadGameMemory(
            CITY_BONDS_RVA,
            &result.city_bonds,
            sizeof(result.city_bonds)
        )) {
        return false;
    }

    if (!TryReadGameMemory(
            CITY_LAND_VALUE_RVA,
            &result.city_land_value,
            sizeof(result.city_land_value)
        )) {
        return false;
    }

    if (!TryReadGameMemory(
            CITY_VALUE_RVA,
            &result.city_value,
            sizeof(result.city_value)
        )) {
        return false;
    }

    if (!TryReadGameMemory(
            INTEREST_RATE_SUM_RVA,
            &result.interest_rate_sum,
            sizeof(result.interest_rate_sum)
        )) {
        return false;
    }

    snapshot = result;
    return true;
}

}
