#include "neomaxis/bond_analytics.h"

#include <limits>

namespace neomaxis {

namespace {

std::uint32_t CalculateRateSum(const BondSnapshot& snapshot)
{
    std::uint32_t sum = 0;
    for (std::size_t i = 0; i < snapshot.count; ++i)
        sum += snapshot.rates[i];
    return sum;
}

bool IsValidEvent(BondAnalyticsEvent event)
{
    switch (event) {
    case BondAnalyticsEvent::Baseline:
    case BondAnalyticsEvent::CountIncreased:
    case BondAnalyticsEvent::CountDecreased:
    case BondAnalyticsEvent::RateChanged:
        return true;
    }
    return false;
}

}

std::int64_t BondAnalyticsSnapshot::NetBondCountChange() const
{
    if (!initialized)
        return 0;

    return static_cast<std::int64_t>(current_bond_count) -
        static_cast<std::int64_t>(initial_bond_count);
}

double BondAnalyticsSnapshot::AverageRateSumRaw() const
{
    if (observations == 0)
        return 0.0;

    return static_cast<double>(cumulative_rate_sum_raw) /
        static_cast<double>(observations);
}

bool BondHistoryAnalytics::Observe(
    const BondSnapshot& snapshot,
    BondAnalyticsEvent event
)
{
    if (snapshot.count > BOND_SLOT_COUNT || !IsValidEvent(event))
        return false;

    if (!analytics_.initialized) {
        if (event != BondAnalyticsEvent::Baseline)
            return false;
    } else if (event == BondAnalyticsEvent::Baseline) {
        return false;
    }

    const std::uint32_t rate_sum = CalculateRateSum(snapshot);

    if (analytics_.observations ==
        std::numeric_limits<std::uint64_t>::max())
        return false;

    if (analytics_.cumulative_rate_sum_raw >
        std::numeric_limits<std::uint64_t>::max() - rate_sum)
        return false;

    BondAnalyticsSnapshot next = analytics_;

    if (!next.initialized) {
        next.initialized = true;
        next.initial_bond_count = snapshot.count;
        next.minimum_bond_count = snapshot.count;
        next.maximum_bond_count = snapshot.count;
        next.minimum_rate_sum_raw = rate_sum;
        next.maximum_rate_sum_raw = rate_sum;
    }

    ++next.observations;

    switch (event) {
    case BondAnalyticsEvent::Baseline:
        break;
    case BondAnalyticsEvent::CountIncreased:
        ++next.issuance_events;
        break;
    case BondAnalyticsEvent::CountDecreased:
        ++next.removal_events;
        break;
    case BondAnalyticsEvent::RateChanged:
        ++next.rate_change_events;
        break;
    }

    next.current_bond_count = snapshot.count;

    if (snapshot.count < next.minimum_bond_count)
        next.minimum_bond_count = snapshot.count;
    if (snapshot.count > next.maximum_bond_count)
        next.maximum_bond_count = snapshot.count;
    if (rate_sum < next.minimum_rate_sum_raw)
        next.minimum_rate_sum_raw = rate_sum;
    if (rate_sum > next.maximum_rate_sum_raw)
        next.maximum_rate_sum_raw = rate_sum;

    next.cumulative_rate_sum_raw += rate_sum;

    // Track only slots active now or in the previous observation.
    const std::size_t tracked_count =
        snapshot.count > previous_.count
            ? snapshot.count
            : previous_.count;

    for (std::size_t i = 0; i < tracked_count; ++i) {
        const std::uint16_t current_rate =
            i < snapshot.count ? snapshot.rates[i] : 0;
        const std::uint16_t previous_rate =
            i < previous_.count ? previous_.rates[i] : 0;

        BondSlotAnalytics& slot = next.slots[i];

        if (!slot.initialized) {
            slot.initialized = true;
            slot.minimum_rate = current_rate;
            slot.maximum_rate = current_rate;
        } else {
            if (current_rate < slot.minimum_rate)
                slot.minimum_rate = current_rate;
            if (current_rate > slot.maximum_rate)
                slot.maximum_rate = current_rate;
            if (current_rate != previous_rate)
                ++slot.changes;
        }

        ++slot.observations;
        slot.last_rate = current_rate;
    }

    analytics_ = next;
    previous_ = snapshot;
    return true;
}

const BondAnalyticsSnapshot& BondHistoryAnalytics::GetSnapshot() const
{
    return analytics_;
}

void BondHistoryAnalytics::Reset()
{
    analytics_ = BondAnalyticsSnapshot{};
    previous_ = BondSnapshot{};
}

}
