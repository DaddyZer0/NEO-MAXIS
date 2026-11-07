#include "neomaxis/budget_diff.h"

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

    BudgetChangeTracker tracker;
    BudgetChangeSet changes{};
    BudgetSnapshot baseline{};

    const std::size_t police =
        static_cast<std::size_t>(BudgetCategory::Police);

    const std::size_t fire =
        static_cast<std::size_t>(BudgetCategory::Fire);

    baseline.categories[police].funding_percent = 100;
    baseline.categories[police].fund_month[0] = 100;

    Check(
        tracker.Update(baseline, changes) == BudgetUpdateResult::InitialSnapshot,
        "first snapshot establishes baseline"
    );

    Check(!changes.Any(), "initial snapshot has no changes");

    Check(
        tracker.Update(baseline, changes) == BudgetUpdateResult::NoChanges,
        "identical snapshot reports no changes"
    );

    BudgetSnapshot funding_changed = baseline;

    funding_changed.categories[police].funding_percent = 80;
    funding_changed.categories[police].fund_month[1] = 80;

    Check(
        tracker.Update(funding_changed, changes) == BudgetUpdateResult::ChangesDetected,
        "funding adjustment is detected"
    );

    Check(changes.CategoryChanged(police), "police category is marked changed");

    Check(
        changes.categories[police].funding_percent_changed,
        "funding percentage change is detected"
    );

    Check(
        changes.categories[police].fund_month_changed,
        "monthly funding history change is detected"
    );

    Check(
        changes.categories[police].fund_month_slots[1],
        "the changed funding history slot is identified"
    );

    Check(
        !changes.categories[police].fund_month_slots[0],
        "an unchanged funding history slot is not marked"
    );

    Check(
        !changes.categories[police].fund_month_slots[2],
        "another unchanged funding history slot is not marked"
    );

    Check(!changes.CategoryChanged(fire), "unchanged fire category stays unchanged");

    BudgetSnapshot costs_changed = funding_changed;

    costs_changed.categories[police].current_costs = 1;
    costs_changed.categories[police].year_to_date_cost = 80;
    costs_changed.categories[police].estimated_cost = 880;
    costs_changed.categories[police].count_month[1] = 1;

    Check(
        tracker.Update(costs_changed, changes) == BudgetUpdateResult::ChangesDetected,
        "cost changes are detected"
    );

    Check(
        changes.categories[police].current_costs_changed,
        "current costs change is detected"
    );

    Check(
        changes.categories[police].year_to_date_cost_changed,
        "year-to-date cost change is detected"
    );

    Check(
        changes.categories[police].estimated_cost_changed,
        "estimated cost change is detected"
    );

    Check(
        changes.categories[police].count_month_changed,
        "monthly count history change is detected"
    );

    Check(
        changes.categories[police].count_month_slots[1],
        "the changed count history slot is identified"
    );

    Check(
        !changes.categories[police].count_month_slots[0],
        "an unchanged count history slot is not marked"
    );

    BudgetSnapshot second_category = costs_changed;
    second_category.categories[fire].funding_percent = 75;

    Check(
        tracker.Update(second_category, changes) == BudgetUpdateResult::ChangesDetected,
        "second category change is detected"
    );

    Check(changes.CategoryChanged(fire), "fire category is marked changed");
    Check(!changes.CategoryChanged(police), "unchanged police category is not marked");
    Check(!changes.CategoryChanged(BUDGET_CATEGORY_COUNT), "invalid index is safe");

    tracker.Reset();

    Check(
        tracker.Update(second_category, changes) == BudgetUpdateResult::InitialSnapshot,
        "reset establishes a new baseline"
    );

    std::cout
        << "Budget diff checks: " << checks
        << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
