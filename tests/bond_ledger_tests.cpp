#include "neomaxis/bond_ledger.h"

#include <cstdlib>
#include <cstring>
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

    char output[2048]{};

    Check(
        FormatBondLedgerHeader(output, sizeof(output)),
        "CSV header formats"
    );

    Check(
        std::strcmp(
            output,
            "\"SimulationDate\",ElapsedDays,Event,"
            "PreviousCount,CurrentCount,"
            "PreviousRateSumRaw,CurrentRateSumRaw,"
            "\"CurrentRates\",\"ChangedSlots\"\r\n"
        ) == 0,
        "CSV header columns are correct"
    );

    BondSnapshot baseline{};
    baseline.count = 1;
    baseline.rates[0] = 4;

    CalendarDate first_date{1900, 1, 1, 0, 0};

    Check(
        FormatBondLedgerRow(
            first_date, true, baseline, BondChangeSet{},
            output, sizeof(output)),
        "baseline row formats"
    );

    Check(
        std::strcmp(
            output,
            "\"January 1, 1900\",0,baseline,,1,,4,\"4\",\"\"\r\n"
        ) == 0,
        "baseline row preserves empty previous fields"
    );

    BondChangeSet changes{};
    changes.previous_count = 1;
    changes.current_count = 2;
    changes.previous_rate_sum = 4;
    changes.current_rate_sum = 11;
    changes.count_increased = true;
    changes.rate_slots[1] = true;
    changes.previous_slot_rates[1] = 0;
    changes.current_slot_rates[1] = 7;

    BondSnapshot issued = baseline;
    issued.count = 2;
    issued.rates[1] = 7;

    CalendarDate second_date{1900, 1, 2, 1, 0};

    Check(
        FormatBondLedgerRow(
            second_date, false, issued, changes,
            output, sizeof(output)),
        "issuance row formats"
    );

    Check(
        std::strcmp(
            output,
            "\"January 2, 1900\",1,count_increased,"
            "1,2,4,11,\"4;7\",\"1:0->7\"\r\n"
        ) == 0,
        "issuance row records slot transition"
    );

    changes.count_increased = false;
    changes.previous_count = 2;
    changes.current_count = 2;
    changes.previous_rate_sum = 11;
    changes.current_rate_sum = 13;
    changes.rate_slots.fill(false);
    changes.rate_slots[0] = true;
    changes.previous_slot_rates[0] = 4;
    changes.current_slot_rates[0] = 6;

    BondSnapshot modified = issued;
    modified.rates[0] = 6;

    Check(
        FormatBondLedgerRow(
            second_date, false, modified, changes,
            output, sizeof(output)),
        "rate modification row formats"
    );

    Check(
        std::strstr(output, "rate_changed") != nullptr &&
        std::strstr(output, "\"0:4->6\"") != nullptr,
        "rate modification is recorded"
    );

    changes.count_decreased = true;
    changes.previous_count = 2;
    changes.current_count = 1;
    changes.previous_rate_sum = 13;
    changes.current_rate_sum = 6;
    changes.rate_slots.fill(false);
    changes.rate_slots[1] = true;
    changes.previous_slot_rates[1] = 7;
    changes.current_slot_rates[1] = 0;

    BondSnapshot removed = modified;
    removed.count = 1;

    Check(
        FormatBondLedgerRow(
            second_date, false, removed, changes,
            output, sizeof(output)),
        "removal row formats"
    );

    Check(
        std::strstr(output, "count_decreased") != nullptr &&
        std::strstr(output, "\"1:7->0\"") != nullptr,
        "removed slot is recorded"
    );

    Check(!FormatBondLedgerHeader(nullptr, sizeof(output)),
        "null header buffer rejected");

    Check(!FormatBondLedgerHeader(output, 0),
        "zero-sized header buffer rejected");

    Check(!FormatBondLedgerRow(
        first_date, true, baseline, BondChangeSet{},
        output, 1),
        "undersized row buffer rejected");

    Check(!FormatBondLedgerRow(
        first_date, true, baseline, BondChangeSet{},
        nullptr, sizeof(output)),
        "null row buffer rejected");

    BondSnapshot invalid{};
    invalid.count = BOND_SLOT_COUNT + 1;

    Check(!FormatBondLedgerRow(
        first_date, true, invalid, BondChangeSet{},
        output, sizeof(output)),
        "invalid bond count rejected");

    std::cout
        << "Bond ledger checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
