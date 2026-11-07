#include "neomaxis/bond_analytics.h"

#include <cmath>
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

    BondHistoryAnalytics history;
    BondSnapshot snapshot{};

    Check(!history.GetSnapshot().initialized,
        "analytics starts uninitialized");
    Check(!history.Observe(snapshot, BondAnalyticsEvent::CountIncreased),
        "non-baseline first observation rejected");
    Check(history.Observe(snapshot, BondAnalyticsEvent::Baseline),
        "baseline observation accepted");

    snapshot.count = 1;
    snapshot.rates[0] = 4;
    Check(history.Observe(snapshot, BondAnalyticsEvent::CountIncreased),
        "issuance observation accepted");

    snapshot.rates[0] = 6;
    Check(history.Observe(snapshot, BondAnalyticsEvent::RateChanged),
        "rate change observation accepted");

    snapshot.count = 0;
    snapshot.rates[0] = 0;
    Check(history.Observe(snapshot, BondAnalyticsEvent::CountDecreased),
        "removal observation accepted");

    const BondAnalyticsSnapshot& result = history.GetSnapshot();

    Check(result.observations == 4, "four observations recorded");
    Check(result.issuance_events == 1, "one issuance event recorded");
    Check(result.removal_events == 1, "one removal event recorded");
    Check(result.rate_change_events == 1, "one rate change event recorded");
    Check(result.initial_bond_count == 0, "initial count preserved");
    Check(result.current_bond_count == 0, "current count updated");
    Check(result.minimum_bond_count == 0, "minimum count tracked");
    Check(result.maximum_bond_count == 1, "maximum count tracked");
    Check(result.NetBondCountChange() == 0, "net count change calculated");
    Check(result.minimum_rate_sum_raw == 0, "minimum raw sum tracked");
    Check(result.maximum_rate_sum_raw == 6, "maximum raw sum tracked");
    Check(result.cumulative_rate_sum_raw == 10, "cumulative raw sum calculated");
    Check(std::fabs(result.AverageRateSumRaw() - 2.5) < 0.000001,
        "average raw sum calculated");

    Check(result.slots[0].observations == 3,
        "slot observations include active states and removal");
    Check(result.slots[0].minimum_rate == 0, "slot minimum tracked");
    Check(result.slots[0].maximum_rate == 6, "slot maximum tracked");
    Check(result.slots[0].last_rate == 0, "slot latest rate tracked");
    Check(result.slots[0].changes == 2, "slot transitions tracked");
    Check(!result.slots[1].initialized,
        "unused slots are not recorded as zero-rate bonds");

    Check(!history.Observe(snapshot, BondAnalyticsEvent::Baseline),
        "duplicate baseline rejected");

    BondSnapshot invalid{};
    invalid.count = BOND_SLOT_COUNT + 1;
    Check(!history.Observe(invalid, BondAnalyticsEvent::RateChanged),
        "invalid snapshot rejected");
    Check(history.GetSnapshot().observations == 4,
        "rejected observations do not alter history");

    history.Reset();
    Check(!history.GetSnapshot().initialized, "reset clears analytics");
    Check(history.Observe(snapshot, BondAnalyticsEvent::Baseline),
        "new session baseline accepted");

    std::cout << "Bond analytics checks: " << checks
              << ", failures: " << failures << '\n';
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
