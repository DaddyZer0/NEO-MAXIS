#pragma once

#include <cstdint>

namespace neomaxis {

struct EconomySnapshot {
    std::int32_t city_funds = 0;
    std::int32_t city_bonds = 0;
    std::int32_t city_land_value = 0;
    std::uint32_t city_value = 0;
    std::uint32_t interest_rate_sum = 0;
};

static_assert(sizeof(EconomySnapshot) == 20,
    "EconomySnapshot layout changed unexpectedly.");

}
