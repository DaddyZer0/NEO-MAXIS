#include "neomaxis/bond_reader.h"

#include <array>
#include <cstddef>
#include <cstdint>

#include "neomaxis/game_memory.h"

namespace neomaxis {

namespace {

constexpr std::uintptr_t BOND_RATE_ARRAY_RVA = 0x000C9EF0;
constexpr std::uintptr_t CITY_BOND_COUNT_RVA = 0x000CC4E8;

}

bool TryReadBondSnapshot(BondSnapshot& snapshot)
{
    std::int32_t initial_count = 0;

    if (!TryReadGameMemory(
            CITY_BOND_COUNT_RVA,
            &initial_count,
            sizeof(initial_count))) {
        return false;
    }

    if (!IsValidBondCount(initial_count))
        return false;

    std::array<std::uint16_t, BOND_SLOT_COUNT> raw_rates{};

    const std::size_t active_size =
        static_cast<std::size_t>(initial_count) *
        sizeof(std::uint16_t);

    if (active_size > 0) {
        if (!TryReadGameMemory(
                BOND_RATE_ARRAY_RVA,
                raw_rates.data(),
                active_size)) {
            return false;
        }
    }

    std::int32_t final_count = 0;

    if (!TryReadGameMemory(
            CITY_BOND_COUNT_RVA,
            &final_count,
            sizeof(final_count))) {
        return false;
    }

    if (initial_count != final_count)
        return false;

    BondSnapshot result{};

    if (!TryDecodeBondSnapshot(
            initial_count,
            raw_rates.data(),
            active_size,
            result)) {
        return false;
    }

    snapshot = result;
    return true;
}

}
