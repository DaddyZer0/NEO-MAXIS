#include "neomaxis/budget.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace {

int failures = 0;

void Check(bool condition, const char* description)
{
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failures;
    }
}

}

int main()
{
    using namespace neomaxis;

    Check(
        sizeof(BudgetRecord) == 112,
        "BudgetRecord is 112 bytes"
    );

    Check(
        BUDGET_CATEGORY_COUNT == 16,
        "There are 16 budget categories"
    );

    Check(
        sizeof(BudgetSnapshot) == 1792,
        "BudgetSnapshot is 1792 bytes"
    );

    std::array<std::uint8_t, sizeof(BudgetSnapshot)> raw{};

    BudgetRecord expected{};

    expected.count_month[0] = 125;
    expected.count_month[11] = 900;

    expected.fund_month[0] = 75;
    expected.fund_month[11] = 850;

    expected.current_costs = 1200;
    expected.funding_percent = 100;
    expected.year_to_date_cost = 8400;
    expected.estimated_cost = 10000;

    std::memcpy(
        raw.data(),
        &expected,
        sizeof(expected)
    );

    BudgetSnapshot decoded{};

    Check(
        TryDecodeBudgetSnapshot(
            raw.data(),
            raw.size(),
            decoded
        ),
        "Valid budget snapshot decodes"
    );

    const BudgetRecord& first =
        decoded.categories[0];

    Check(
        first.count_month[0] == 125,
        "First monthly count preserved"
    );

    Check(
        first.count_month[11] == 900,
        "Last monthly count preserved"
    );

    Check(
        first.fund_month[0] == 75,
        "First monthly funding preserved"
    );

    Check(
        first.fund_month[11] == 850,
        "Last monthly funding preserved"
    );

    Check(
        first.current_costs == 1200,
        "Current costs preserved"
    );

    Check(
        first.funding_percent == 100,
        "Funding percentage preserved"
    );

    Check(
        first.year_to_date_cost == 8400,
        "Year-to-date cost preserved"
    );

    Check(
        first.estimated_cost == 10000,
        "Estimated cost preserved"
    );

    Check(
        !TryDecodeBudgetSnapshot(
            nullptr,
            raw.size(),
            decoded
        ),
        "Null source rejected"
    );

    Check(
        !TryDecodeBudgetSnapshot(
            raw.data(),
            raw.size() - 1,
            decoded
        ),
        "Undersized source rejected"
    );

    Check(
        !TryDecodeBudgetSnapshot(
            raw.data(),
            raw.size() + 1,
            decoded
        ),
        "Oversized source rejected"
    );

    std::cout << "\nPassed: " << (12 - failures)
              << "\nFailed: " << failures
              << "\nTotal: 12\n";

    return failures == 0 ? 0 : 1;
}
