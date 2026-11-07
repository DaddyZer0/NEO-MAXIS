#include "neomaxis/budget_reader.h"

#include <array>
#include <cstdint>

#include "neomaxis/game_memory.h"

namespace neomaxis {

namespace {

constexpr std::uintptr_t BUDGET_POINTER_RVA = 0x000CC4CC;

}

bool TryReadBudgetSnapshot(BudgetSnapshot& snapshot)
{
    std::uintptr_t budget_address = 0;

    if (!TryReadGamePointer(
            BUDGET_POINTER_RVA,
            budget_address
        )) {
        return false;
    }

    std::array<std::uint8_t, sizeof(BudgetSnapshot)> raw_data{};

    if (!TryReadGameAddress(
            budget_address,
            raw_data.data(),
            raw_data.size()
        )) {
        return false;
    }

    BudgetSnapshot result{};

    if (!TryDecodeBudgetSnapshot(
            raw_data.data(),
            raw_data.size(),
            result
        )) {
        return false;
    }

    snapshot = result;
    return true;
}

}
