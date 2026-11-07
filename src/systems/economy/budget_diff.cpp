#include "neomaxis/budget_diff.h"

namespace neomaxis {
namespace {

BudgetCategoryChange CompareCategory(
    const BudgetRecord& previous,
    const BudgetRecord& current
)
{
    BudgetCategoryChange change{};

    for (std::size_t month = 0; month < 12; ++month) {
        change.count_month_slots[month] =
            previous.count_month[month] != current.count_month[month];

        change.fund_month_slots[month] =
            previous.fund_month[month] != current.fund_month[month];

        change.count_month_changed =
            change.count_month_changed ||
            change.count_month_slots[month];

        change.fund_month_changed =
            change.fund_month_changed ||
            change.fund_month_slots[month];
    }

    change.current_costs_changed =
        previous.current_costs != current.current_costs;

    change.funding_percent_changed =
        previous.funding_percent != current.funding_percent;

    change.year_to_date_cost_changed =
        previous.year_to_date_cost != current.year_to_date_cost;

    change.estimated_cost_changed =
        previous.estimated_cost != current.estimated_cost;

    return change;
}

} // namespace

bool BudgetCategoryChange::Any() const
{
    return count_month_changed ||
        fund_month_changed ||
        current_costs_changed ||
        funding_percent_changed ||
        year_to_date_cost_changed ||
        estimated_cost_changed;
}

bool BudgetChangeSet::Any() const
{
    for (const auto& category : categories) {
        if (category.Any())
            return true;
    }

    return false;
}

bool BudgetChangeSet::CategoryChanged(std::size_t index) const
{
    return index < categories.size() && categories[index].Any();
}

BudgetUpdateResult BudgetChangeTracker::Update(
    const BudgetSnapshot& current,
    BudgetChangeSet& changes
)
{
    BudgetChangeSet result{};

    if (!has_previous_) {
        previous_ = current;
        has_previous_ = true;
        changes = result;
        return BudgetUpdateResult::InitialSnapshot;
    }

    for (std::size_t index = 0; index < BUDGET_CATEGORY_COUNT; ++index) {
        result.categories[index] = CompareCategory(
            previous_.categories[index],
            current.categories[index]
        );
    }

    previous_ = current;
    changes = result;

    return result.Any()
        ? BudgetUpdateResult::ChangesDetected
        : BudgetUpdateResult::NoChanges;
}

void BudgetChangeTracker::Reset()
{
    previous_ = BudgetSnapshot{};
    has_previous_ = false;
}

} // namespace neomaxis
