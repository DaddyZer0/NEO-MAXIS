#pragma once

#include <array>
#include <cstddef>
#include "neomaxis/budget.h"

namespace neomaxis {

enum class BudgetUpdateResult {
    InitialSnapshot,
    NoChanges,
    ChangesDetected
};

struct BudgetCategoryChange {
    std::array<bool, 12> count_month_slots{};
    std::array<bool, 12> fund_month_slots{};

    bool count_month_changed = false;
    bool fund_month_changed = false;
    bool current_costs_changed = false;
    bool funding_percent_changed = false;
    bool year_to_date_cost_changed = false;
    bool estimated_cost_changed = false;

    bool Any() const;
};

struct BudgetChangeSet {
    std::array<BudgetCategoryChange, BUDGET_CATEGORY_COUNT> categories{};

    bool Any() const;
    bool CategoryChanged(std::size_t index) const;
};

class BudgetChangeTracker {
public:
    BudgetUpdateResult Update(
        const BudgetSnapshot& current,
        BudgetChangeSet& changes
    );

    void Reset();

private:
    bool has_previous_ = false;
    BudgetSnapshot previous_{};
};

} // namespace neomaxis
