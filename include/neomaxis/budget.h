#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace neomaxis {

enum class BudgetCategory : std::size_t {
    Residential = 0,
    Commercial,
    Industrial,
    Ordinances,
    Bonds,
    Police,
    Fire,
    Health,
    School,
    College,
    Road,
    Highway,
    Bridge,
    Rail,
    Subway,
    Tunnel,
    Count
};

constexpr std::size_t BUDGET_CATEGORY_COUNT =
    static_cast<std::size_t>(BudgetCategory::Count);

struct BudgetRecord {
    std::int32_t count_month[12]{};
    std::int32_t fund_month[12]{};

    std::int32_t current_costs = 0;
    std::int32_t funding_percent = 0;
    std::int32_t year_to_date_cost = 0;
    std::int32_t estimated_cost = 0;
};

struct BudgetSnapshot {
    std::array<BudgetRecord, BUDGET_CATEGORY_COUNT> categories{};
};

static_assert(sizeof(std::int32_t) == 4);
static_assert(sizeof(BudgetRecord) == 112);
static_assert(BUDGET_CATEGORY_COUNT == 16);
static_assert(sizeof(BudgetSnapshot) == 1792);

bool TryDecodeBudgetSnapshot(
    const void* source,
    std::size_t source_size,
    BudgetSnapshot& snapshot
);

}
