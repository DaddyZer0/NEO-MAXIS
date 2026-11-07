#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace neomaxis {

constexpr std::size_t BOND_SLOT_COUNT = 50;

struct BondSnapshot {
    std::uint32_t count = 0;
    std::array<std::uint16_t, BOND_SLOT_COUNT> rates{};
};

static_assert(sizeof(BondSnapshot) == 104,
    "BondSnapshot layout changed unexpectedly.");

bool IsValidBondCount(std::int32_t count);

bool TryDecodeBondSnapshot(
    std::int32_t count,
    const void* source,
    std::size_t source_size,
    BondSnapshot& snapshot
);

}
