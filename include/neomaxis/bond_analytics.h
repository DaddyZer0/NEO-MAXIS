#pragma once

#include <array>
#include <cstdint>

#include "neomaxis/bond.h"

namespace neomaxis {

enum class BondAnalyticsEvent {
    Baseline,
    CountIncreased,
    CountDecreased,
    RateChanged
};

struct BondSlotAnalytics {
    std::uint32_t observations = 0;
    std::uint32_t changes = 0;
    std::uint16_t minimum_rate = 0;
    std::uint16_t maximum_rate = 0;
    std::uint16_t last_rate = 0;
    bool initialized = false;
};

struct BondAnalyticsSnapshot {
    bool initialized = false;
    std::uint64_t observations = 0;
    std::uint64_t issuance_events = 0;
    std::uint64_t removal_events = 0;
    std::uint64_t rate_change_events = 0;
    std::uint32_t initial_bond_count = 0;
    std::uint32_t current_bond_count = 0;
    std::uint32_t minimum_bond_count = 0;
    std::uint32_t maximum_bond_count = 0;
    std::uint32_t minimum_rate_sum_raw = 0;
    std::uint32_t maximum_rate_sum_raw = 0;
    std::uint64_t cumulative_rate_sum_raw = 0;
    std::array<BondSlotAnalytics, BOND_SLOT_COUNT> slots{};

    std::int64_t NetBondCountChange() const;
    double AverageRateSumRaw() const;
};

class BondHistoryAnalytics {
public:
    bool Observe(const BondSnapshot& snapshot, BondAnalyticsEvent event);
    const BondAnalyticsSnapshot& GetSnapshot() const;
    void Reset();

private:
    BondAnalyticsSnapshot analytics_{};
    BondSnapshot previous_{};
};

}
