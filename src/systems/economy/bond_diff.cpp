#include "neomaxis/bond_diff.h"

namespace neomaxis {

namespace {

std::uint32_t CalculateRateSum(const BondSnapshot& snapshot)
{
    std::uint32_t sum = 0;

    for (std::size_t i = 0; i < snapshot.count; ++i)
        sum += snapshot.rates[i];

    return sum;
}

BondChangeSet CompareSnapshots(
    const BondSnapshot& previous,
    const BondSnapshot& current
)
{
    BondChangeSet changes{};

    changes.previous_count = previous.count;
    changes.current_count = current.count;

    changes.count_changed =
        previous.count != current.count;

    changes.count_increased =
        current.count > previous.count;

    changes.count_decreased =
        current.count < previous.count;

    changes.previous_rate_sum = CalculateRateSum(previous);
    changes.current_rate_sum = CalculateRateSum(current);

    const std::size_t comparison_count =
        previous.count > current.count
            ? previous.count
            : current.count;

    for (std::size_t i = 0; i < comparison_count; ++i) {
        const std::uint16_t previous_rate =
            i < previous.count ? previous.rates[i] : 0;

        const std::uint16_t current_rate =
            i < current.count ? current.rates[i] : 0;

        changes.previous_slot_rates[i] = previous_rate;
        changes.current_slot_rates[i] = current_rate;
        changes.rate_slots[i] =
            previous_rate != current_rate;
    }

    for (bool changed : changes.rate_slots) {
        if (changed) {
            changes.rates_changed = true;
            break;
        }
    }

    return changes;
}

}

bool BondChangeSet::Any() const
{
    return count_changed || rates_changed;
}

bool BondChangeSet::RateSlotChanged(std::size_t index) const
{
    return index < rate_slots.size() && rate_slots[index];
}

BondChangeResult BondChangeTracker::Update(
    const BondSnapshot& current,
    BondChangeSet& changes
)
{
    BondChangeSet result{};

    if (!has_previous_) {
        previous_ = current;
        has_previous_ = true;
        changes = result;

        return BondChangeResult::InitialSnapshot;
    }

    result = CompareSnapshots(previous_, current);

    previous_ = current;
    changes = result;

    return result.Any()
        ? BondChangeResult::ChangesDetected
        : BondChangeResult::NoChanges;
}

void BondChangeTracker::Reset()
{
    previous_ = BondSnapshot{};
    has_previous_ = false;
}

}
