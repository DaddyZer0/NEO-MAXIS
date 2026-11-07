#include "neomaxis/bond_diff.h"

#include <cstdlib>
#include <iostream>

namespace {

int checks = 0;
int failures = 0;

void Check(bool condition, const char* description)
{
    ++checks;

    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << description << '\n';
    }
}

}

int main()
{
    using namespace neomaxis;

    BondChangeTracker tracker;
    BondChangeSet changes{};

    BondSnapshot baseline{};
    baseline.count = 1;
    baseline.rates[0] = 4;

    Check(
        tracker.Update(baseline, changes) ==
            BondChangeResult::InitialSnapshot,
        "first snapshot establishes baseline"
    );

    Check(!changes.Any(), "initial snapshot has no changes");

    Check(
        tracker.Update(baseline, changes) ==
            BondChangeResult::NoChanges,
        "identical snapshot reports no changes"
    );

    BondSnapshot second_bond = baseline;
    second_bond.count = 2;
    second_bond.rates[1] = 7;

    Check(
        tracker.Update(second_bond, changes) ==
            BondChangeResult::ChangesDetected,
        "additional bond is detected"
    );

    Check(changes.count_changed, "bond count change detected");
    Check(changes.count_increased, "bond count increase detected");
    Check(!changes.count_decreased, "increase is not a decrease");

    Check(changes.previous_count == 1, "previous count preserved");
    Check(changes.current_count == 2, "current count preserved");

    Check(changes.rates_changed, "rate array change detected");
    Check(changes.RateSlotChanged(1), "new rate slot identified");
    Check(changes.previous_slot_rates[1] == 0,
        "new slot previous value is zero");
    Check(changes.current_slot_rates[1] == 7,
        "new slot current value is preserved");
    Check(!changes.RateSlotChanged(0), "unchanged rate slot preserved");

    Check(changes.previous_rate_sum == 4, "previous rate sum calculated");
    Check(changes.current_rate_sum == 11, "current rate sum calculated");

    BondSnapshot modified_rate = second_bond;
    modified_rate.rates[0] = 6;

    Check(
        tracker.Update(modified_rate, changes) ==
            BondChangeResult::ChangesDetected,
        "individual rate modification detected"
    );

    Check(!changes.count_changed, "rate modification preserves count");
    Check(changes.RateSlotChanged(0), "modified rate slot identified");
    Check(changes.previous_slot_rates[0] == 4,
        "modified slot previous value is preserved");
    Check(changes.current_slot_rates[0] == 6,
        "modified slot current value is preserved");
    Check(changes.rates_changed, "modified rate array detected");

    Check(changes.previous_rate_sum == 11, "previous sum before modification");
    Check(changes.current_rate_sum == 13, "new sum after modification");

    BondSnapshot reduced = modified_rate;
    reduced.count = 1;

    Check(
        tracker.Update(reduced, changes) ==
            BondChangeResult::ChangesDetected,
        "reduced bond count detected"
    );

    Check(changes.count_decreased, "bond count decrease detected");
    Check(!changes.count_increased, "decrease is not an increase");
    Check(changes.RateSlotChanged(1), "removed rate slot identified");
    Check(changes.previous_slot_rates[1] == 7,
        "removed slot previous value is preserved");
    Check(changes.current_slot_rates[1] == 0,
        "removed slot current value is zero");

    Check(changes.previous_count == 2, "decrease previous count");
    Check(changes.current_count == 1, "decrease current count");

    Check(
        !changes.RateSlotChanged(BOND_SLOT_COUNT),
        "invalid rate slot index is safe"
    );

    tracker.Reset();

    Check(
        tracker.Update(reduced, changes) ==
            BondChangeResult::InitialSnapshot,
        "reset establishes a new baseline"
    );

    Check(!changes.Any(), "reset baseline has no changes");

    std::cout
        << "Bond diff checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
