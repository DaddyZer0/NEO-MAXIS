#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <string.h>

#include "sc2kfix.h"
#include "neomaxis/calendar.h"
#include "neomaxis/economy_reader.h"
#include "neomaxis/budget_reader.h"
#include "neomaxis/bond_reader.h"
#include "neomaxis/bond_diff.h"
#include "neomaxis/bond_ledger.h"
#include "neomaxis/bond_analytics.h"
#include "neomaxis/budget_diff.h"
#include "neomaxis/game_state.h"

namespace {

std::uint32_t last_reported_day = 0;
std::uint16_t last_start_year = 0;
bool has_reported_state = false;

neomaxis::BudgetChangeTracker budget_tracker;
bool has_logged_budget_failure = false;

neomaxis::BondChangeTracker bond_tracker;
neomaxis::BondHistoryAnalytics bond_history;
bool has_logged_bond_failure = false;
bool has_logged_analytics_failure = false;

constexpr const char* BUDGET_CATEGORY_NAMES[] = {
    "Residential",
    "Commercial",
    "Industrial",
    "Ordinances",
    "Bonds",
    "Police",
    "Fire",
    "Health",
    "School",
    "College",
    "Road",
    "Highway",
    "Bridge",
    "Rail",
    "Subway",
    "Tunnel"
};

static_assert(
    sizeof(BUDGET_CATEGORY_NAMES) /
    sizeof(BUDGET_CATEGORY_NAMES[0]) ==
    neomaxis::BUDGET_CATEGORY_COUNT,
    "Budget category names must match the budget structure."
);

void AppendLog(const char* filename, const char* message)
{
    if (filename == nullptr || message == nullptr)
        return;

    HANDLE file = CreateFileA(
        filename,
        FILE_APPEND_DATA,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE)
        return;

    DWORD bytes_written = 0;
    const DWORD length = static_cast<DWORD>(strlen(message));

    WriteFile(
        file,
        message,
        length,
        &bytes_written,
        nullptr
    );

    CloseHandle(file);
}

bool AppendText(
    char* output,
    std::size_t capacity,
    std::size_t& used,
    const char* text
)
{
    if (output == nullptr || text == nullptr || used >= capacity)
        return false;

    const int written = snprintf(
        output + used,
        capacity - used,
        "%s",
        text
    );

    if (written < 0 ||
        static_cast<std::size_t>(written) >= capacity - used) {
        return false;
    }

    used += static_cast<std::size_t>(written);
    return true;
}

bool AppendValue(
    char* output,
    std::size_t capacity,
    std::size_t& used,
    const char* label,
    std::int32_t value
)
{
    if (output == nullptr || label == nullptr || used >= capacity)
        return false;

    const int written = snprintf(
        output + used,
        capacity - used,
        " | %s=%ld",
        label,
        static_cast<long>(value)
    );

    if (written < 0 ||
        static_cast<std::size_t>(written) >= capacity - used) {
        return false;
    }

    used += static_cast<std::size_t>(written);
    return true;
}

bool AppendChangedMonthSlots(
    char* output,
    std::size_t capacity,
    std::size_t& used,
    const char* label,
    const std::array<bool, 12>& changed_slots,
    const std::int32_t* values
)
{
    if (output == nullptr || label == nullptr || values == nullptr)
        return false;

    for (std::size_t month = 0; month < 12; ++month) {
        if (!changed_slots[month])
            continue;

        if (used >= capacity)
            return false;

        const int written = snprintf(
            output + used,
            capacity - used,
            " | %s[%lu]=%ld",
            label,
            static_cast<unsigned long>(month),
            static_cast<long>(values[month])
        );

        if (written < 0 ||
            static_cast<std::size_t>(written) >= capacity - used) {
            return false;
        }

        used += static_cast<std::size_t>(written);
    }

    return true;
}

void WriteCalendarLog(const neomaxis::CalendarDate& date)
{
    char date_text[64]{};

    if (!neomaxis::FormatCalendarDate(
            date, date_text, sizeof(date_text))) {
        return;
    }

    char message[160]{};

    const int length = snprintf(
        message,
        sizeof(message),
        "NEO-MAXIS: %s | Elapsed days: %lu\r\n",
        date_text,
        static_cast<unsigned long>(date.elapsed_days)
    );

    if (length < 0 ||
        static_cast<std::size_t>(length) >= sizeof(message)) {
        return;
    }

    AppendLog("neomaxis-calendar.log", message);
}

void WriteEconomyLog()
{
    neomaxis::EconomySnapshot economy{};

    if (!neomaxis::TryReadEconomySnapshot(economy)) {
        AppendLog(
            "neomaxis-economy.log",
            "NEO-MAXIS: Economy memory read failed.\r\n"
        );
        return;
    }

    char message[320]{};

    const int length = snprintf(
        message,
        sizeof(message),
        "NEO-MAXIS: Treasury raw=%ld | Bonds raw=%ld | "
        "LandValue raw=%ld | CityValue raw=%lu | "
        "InterestRateSum raw=%lu\r\n",
        static_cast<long>(economy.city_funds),
        static_cast<long>(economy.city_bonds),
        static_cast<long>(economy.city_land_value),
        static_cast<unsigned long>(economy.city_value),
        static_cast<unsigned long>(economy.interest_rate_sum)
    );

    if (length < 0 ||
        static_cast<std::size_t>(length) >= sizeof(message)) {
        return;
    }

    AppendLog("neomaxis-economy.log", message);
}

void WriteBondLedger(
    const neomaxis::CalendarDate& date,
    bool initial,
    const neomaxis::BondSnapshot& snapshot,
    const neomaxis::BondChangeSet& changes
)
{
    char header[256]{};
    char row[2048]{};

    if (!neomaxis::FormatBondLedgerHeader(header, sizeof(header)))
        return;

    if (!neomaxis::FormatBondLedgerRow(
            date, initial, snapshot, changes, row, sizeof(row))) {
        return;
    }

    HANDLE file = CreateFileA(
        "neomaxis-bond-ledger.csv",
        GENERIC_READ | FILE_APPEND_DATA,
        FILE_SHARE_READ,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE)
        return;

    LARGE_INTEGER file_size{};

    if (!GetFileSizeEx(file, &file_size)) {
        CloseHandle(file);
        return;
    }

    bool success = true;
    DWORD bytes_written = 0;

    if (file_size.QuadPart == 0) {
        const DWORD header_length =
            static_cast<DWORD>(strlen(header));

        success = WriteFile(
            file,
            header,
            header_length,
            &bytes_written,
            nullptr
        ) && bytes_written == header_length;
    }

    if (success) {
        const DWORD row_length =
            static_cast<DWORD>(strlen(row));

        success = WriteFile(
            file,
            row,
            row_length,
            &bytes_written,
            nullptr
        ) && bytes_written == row_length;
    }

    CloseHandle(file);
}


void WriteBondHistoryLog(
    const neomaxis::CalendarDate& date,
    const char* event_name
)
{
    const neomaxis::BondAnalyticsSnapshot& history =
        bond_history.GetSnapshot();

    char date_text[64]{};

    if (!neomaxis::FormatCalendarDate(
            date, date_text, sizeof(date_text))) {
        return;
    }

    char message[1024]{};

    const int length = snprintf(
        message,
        sizeof(message),
        "NEO-MAXIS: Bond history | Date=%s | Event=%s"
        " | Observations=%llu"
        " | IssuanceEvents=%llu"
        " | RemovalEvents=%llu"
        " | RateChangeEvents=%llu"
        " | InitialCount=%lu"
        " | CurrentCount=%lu"
        " | NetCountChange=%lld"
        " | MinimumCount=%lu"
        " | MaximumCount=%lu"
        " | MinimumRateSumRaw=%lu"
        " | MaximumRateSumRaw=%lu"
        " | CumulativeRateSumRaw=%llu"
        " | AverageRateSumRaw=%.3f\r\n",
        date_text,
        event_name,
        static_cast<unsigned long long>(history.observations),
        static_cast<unsigned long long>(history.issuance_events),
        static_cast<unsigned long long>(history.removal_events),
        static_cast<unsigned long long>(history.rate_change_events),
        static_cast<unsigned long>(history.initial_bond_count),
        static_cast<unsigned long>(history.current_bond_count),
        static_cast<long long>(history.NetBondCountChange()),
        static_cast<unsigned long>(history.minimum_bond_count),
        static_cast<unsigned long>(history.maximum_bond_count),
        static_cast<unsigned long>(history.minimum_rate_sum_raw),
        static_cast<unsigned long>(history.maximum_rate_sum_raw),
        static_cast<unsigned long long>(history.cumulative_rate_sum_raw),
        history.AverageRateSumRaw()
    );

    if (length < 0 ||
        static_cast<std::size_t>(length) >= sizeof(message)) {
        return;
    }

    AppendLog("neomaxis-bond-history.log", message);

    for (std::size_t i = 0; i < history.slots.size(); ++i) {
        const neomaxis::BondSlotAnalytics& slot = history.slots[i];

        if (!slot.initialized)
            continue;

        char slot_message[256]{};

        const int slot_length = snprintf(
            slot_message,
            sizeof(slot_message),
            "NEO-MAXIS: Bond slot history | Date=%s"
            " | Slot=%lu | Observations=%lu | Changes=%lu"
            " | MinimumRate=%u | MaximumRate=%u"
            " | LastRate=%u\r\n",
            date_text,
            static_cast<unsigned long>(i),
            static_cast<unsigned long>(slot.observations),
            static_cast<unsigned long>(slot.changes),
            static_cast<unsigned int>(slot.minimum_rate),
            static_cast<unsigned int>(slot.maximum_rate),
            static_cast<unsigned int>(slot.last_rate)
        );

        if (slot_length > 0 &&
            static_cast<std::size_t>(slot_length) < sizeof(slot_message)) {
            AppendLog("neomaxis-bond-history.log", slot_message);
        }
    }
}

void WriteBondLog(const neomaxis::CalendarDate& date)
{
    char date_text[64]{};
    if (!neomaxis::FormatCalendarDate(date, date_text, sizeof(date_text)))
        return;

    neomaxis::BondSnapshot snapshot{};

    if (!neomaxis::TryReadBondSnapshot(snapshot)) {
        if (!has_logged_bond_failure) {
            AppendLog(
                "neomaxis-bonds.log",
                "NEO-MAXIS: Bond memory read failed.\r\n"
            );
            has_logged_bond_failure = true;
        }

        return;
    }

    has_logged_bond_failure = false;

    neomaxis::BondChangeSet changes{};

    const neomaxis::BondChangeResult result =
        bond_tracker.Update(snapshot, changes);

    if (result == neomaxis::BondChangeResult::NoChanges)
        return;

    const bool initial =
        result == neomaxis::BondChangeResult::InitialSnapshot;

    std::uint32_t baseline_rate_sum = 0;
    for (std::size_t i = 0; i < snapshot.count; ++i)
        baseline_rate_sum += snapshot.rates[i];

    const char* event_name = "baseline";

    if (!initial) {
        if (changes.count_increased)
            event_name = "count_increased";
        else if (changes.count_decreased)
            event_name = "count_decreased";
        else
            event_name = "rate_changed";
    }

    neomaxis::BondAnalyticsEvent analytics_event =
        neomaxis::BondAnalyticsEvent::Baseline;

    if (!initial) {
        if (changes.count_increased)
            analytics_event = neomaxis::BondAnalyticsEvent::CountIncreased;
        else if (changes.count_decreased)
            analytics_event = neomaxis::BondAnalyticsEvent::CountDecreased;
        else
            analytics_event = neomaxis::BondAnalyticsEvent::RateChanged;
    }

    if (!bond_history.Observe(snapshot, analytics_event)) {
        if (!has_logged_analytics_failure) {
            AppendLog(
                "neomaxis-bond-history.log",
                "NEO-MAXIS: Historical analytics observation failed.\r\n"
            );
            has_logged_analytics_failure = true;
        }
    } else {
        has_logged_analytics_failure = false;
        WriteBondHistoryLog(date, event_name);
    }

    char message[768]{};

    int length = snprintf(
        message,
        sizeof(message),
        "NEO-MAXIS: Bond %s | Date=%s | Count=%lu | Rates=[",
        event_name,
        date_text,
        static_cast<unsigned long>(snapshot.count)
    );

    if (length < 0 ||
        static_cast<std::size_t>(length) >= sizeof(message)) {
        return;
    }

    std::size_t used = static_cast<std::size_t>(length);

    for (std::size_t i = 0; i < snapshot.count; ++i) {
        length = snprintf(
            message + used,
            sizeof(message) - used,
            "%s%u",
            i == 0 ? "" : ",",
            static_cast<unsigned int>(snapshot.rates[i])
        );

        if (length < 0 ||
            static_cast<std::size_t>(length) >= sizeof(message) - used) {
            return;
        }

        used += static_cast<std::size_t>(length);
    }

    if (initial) {
        length = snprintf(
            message + used,
            sizeof(message) - used,
            "] | RateSumRaw=%lu\r\n",
            static_cast<unsigned long>(baseline_rate_sum)
        );
    } else {
        length = snprintf(
            message + used,
            sizeof(message) - used,
            "] | PreviousCount=%lu"
            " | CurrentCount=%lu"
            " | PreviousRateSumRaw=%lu"
            " | CurrentRateSumRaw=%lu"
            " | ChangedSlots=[",
            static_cast<unsigned long>(changes.previous_count),
            static_cast<unsigned long>(changes.current_count),
            static_cast<unsigned long>(changes.previous_rate_sum),
            static_cast<unsigned long>(changes.current_rate_sum)
        );
    }

    if (length < 0 ||
        static_cast<std::size_t>(length) >= sizeof(message) - used) {
        return;
    }

    used += static_cast<std::size_t>(length);

    if (!initial) {
        bool first_slot = true;

        for (std::size_t i = 0; i < changes.rate_slots.size(); ++i) {
            if (!changes.RateSlotChanged(i))
                continue;

            length = snprintf(
                message + used,
                sizeof(message) - used,
                "%s%lu:%u->%u",
                first_slot ? "" : ",",
                static_cast<unsigned long>(i),
                static_cast<unsigned int>(changes.previous_slot_rates[i]),
                static_cast<unsigned int>(changes.current_slot_rates[i])
            );

            if (length < 0 ||
                static_cast<std::size_t>(length) >= sizeof(message) - used) {
                return;
            }

            used += static_cast<std::size_t>(length);
            first_slot = false;
        }

        length = snprintf(
            message + used,
            sizeof(message) - used,
            "]\r\n"
        );

        if (length < 0 ||
            static_cast<std::size_t>(length) >= sizeof(message) - used) {
            return;
        }
    }

    AppendLog("neomaxis-bonds.log", message);

    WriteBondLedger(date, initial, snapshot, changes);
}

void WriteBudgetLog()
{
    neomaxis::BudgetSnapshot snapshot{};

    if (!neomaxis::TryReadBudgetSnapshot(snapshot)) {
        if (!has_logged_budget_failure) {
            AppendLog(
                "neomaxis-budget.log",
                "NEO-MAXIS: Budget snapshot read failed.\r\n"
            );
            has_logged_budget_failure = true;
        }

        return;
    }

    has_logged_budget_failure = false;

    neomaxis::BudgetChangeSet changes{};

    const neomaxis::BudgetUpdateResult result =
        budget_tracker.Update(snapshot, changes);

    if (result == neomaxis::BudgetUpdateResult::InitialSnapshot) {
        AppendLog(
            "neomaxis-budget.log",
            "NEO-MAXIS: Budget baseline initialized.\r\n"
        );
        return;
    }

    if (result == neomaxis::BudgetUpdateResult::NoChanges)
        return;

    for (std::size_t i = 0;
         i < neomaxis::BUDGET_CATEGORY_COUNT;
         ++i) {

        if (!changes.CategoryChanged(i))
            continue;

        const neomaxis::BudgetRecord& record =
            snapshot.categories[i];

        const neomaxis::BudgetCategoryChange& change =
            changes.categories[i];

        char message[768]{};

        const int header_length = snprintf(
            message,
            sizeof(message),
            "NEO-MAXIS: Budget changed | Category=%s",
            BUDGET_CATEGORY_NAMES[i]
        );

        if (header_length < 0 ||
            static_cast<std::size_t>(header_length) >= sizeof(message)) {
            continue;
        }

        std::size_t used =
            static_cast<std::size_t>(header_length);

        bool valid = true;

        if (change.current_costs_changed) {
            valid = AppendValue(
                message, sizeof(message), used,
                "Current", record.current_costs
            );
        }

        if (valid && change.funding_percent_changed) {
            valid = AppendValue(
                message, sizeof(message), used,
                "Funding", record.funding_percent
            );

            if (valid)
                valid = AppendText(message, sizeof(message), used, "%");
        }

        if (valid && change.year_to_date_cost_changed) {
            valid = AppendValue(
                message, sizeof(message), used,
                "YTD", record.year_to_date_cost
            );
        }

        if (valid && change.estimated_cost_changed) {
            valid = AppendValue(
                message, sizeof(message), used,
                "Estimated", record.estimated_cost
            );
        }

        if (valid && change.count_month_changed) {
            valid = AppendChangedMonthSlots(
                message, sizeof(message), used,
                "CountMonth",
                change.count_month_slots,
                record.count_month
            );
        }

        if (valid && change.fund_month_changed) {
            valid = AppendChangedMonthSlots(
                message, sizeof(message), used,
                "FundMonth",
                change.fund_month_slots,
                record.fund_month
            );
        }

        if (!valid)
            continue;

        if (!AppendText(message, sizeof(message), used, "\r\n"))
            continue;

        AppendLog("neomaxis-budget.log", message);
    }
}

} // namespace

HOOKCB void Hook_SimCalendarAdvance_Before(void)
{
    neomaxis::GameState state{};

    if (!neomaxis::TryReadGameState(state))
        return;

    if (has_reported_state &&
        state.elapsed_days == last_reported_day &&
        state.start_year == last_start_year) {
        return;
    }

    const neomaxis::CalendarDate date =
        neomaxis::CalculateCalendarDate(state);

    WriteCalendarLog(date);
    WriteEconomyLog();
    WriteBondLog(date);
    WriteBudgetLog();

    last_reported_day = state.elapsed_days;
    last_start_year = state.start_year;
    has_reported_state = true;
}
