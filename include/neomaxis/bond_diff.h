#pragma once

#include <array>
#include <cstdint>

#include "neomaxis/bond.h"

namespace neomaxis {

enum class BondChangeResult {
    InitialSnapshot,
    NoChanges,
    ChangesDetected
};

struct BondChangeSet {
    std::array<bool, BOND_SLOT_COUNT> rate_slots{};
    std::array<std::uint16_t, BOND_SLOT_COUNT> previous_slot_rates{};
    std::array<std::uint16_t, BOND_SLOT_COUNT> current_slot_rates{};

    bool count_changed = false;
    bool count_increased = false;
    bool count_decreased = false;
    bool rates_changed = false;

    std::uint32_t previous_count = 0;
    std::uint32_t current_count = 0;

    std::uint32_t previous_rate_sum = 0;
    std::uint32_t current_rate_sum = 0;

    bool Any() const;
    bool RateSlotChanged(std::size_t index) const;
};

class BondChangeTracker {
public:
    BondChangeResult Update(
        const BondSnapshot& current,
        BondChangeSet& changes
    );

    void Reset();

private:
    bool has_previous_ = false;
    BondSnapshot previous_{};
};

} // namespace neomaxis
