#include "neomaxis/bond_ledger.h"

#include <cstdarg>
#include <cstdio>

namespace neomaxis {

namespace {

bool AppendFormat(
    char* output,
    std::size_t capacity,
    std::size_t& used,
    const char* format,
    ...
)
{
    if (!output || !format || used >= capacity)
        return false;

    va_list args;
    va_start(args, format);
    const int written = std::vsnprintf(
        output + used, capacity - used, format, args);
    va_end(args);

    if (written < 0 ||
        static_cast<std::size_t>(written) >= capacity - used)
        return false;

    used += static_cast<std::size_t>(written);
    return true;
}

const char* GetEventName(
    bool initial,
    const BondChangeSet& changes)
{
    if (initial) return "baseline";
    if (changes.count_increased) return "count_increased";
    if (changes.count_decreased) return "count_decreased";
    return "rate_changed";
}

std::uint32_t CalculateRateSum(const BondSnapshot& snapshot)
{
    std::uint32_t sum = 0;

    for (std::size_t i = 0; i < snapshot.count; ++i)
        sum += snapshot.rates[i];

    return sum;
}

}

bool FormatBondLedgerHeader(char* output, std::size_t capacity)
{
    if (!output || capacity == 0)
        return false;

    const int written = std::snprintf(
        output, capacity,
        "\"SimulationDate\",ElapsedDays,Event,"
        "PreviousCount,CurrentCount,"
        "PreviousRateSumRaw,CurrentRateSumRaw,"
        "\"CurrentRates\",\"ChangedSlots\"\r\n"
    );

    return written >= 0 &&
        static_cast<std::size_t>(written) < capacity;
}

bool FormatBondLedgerRow(
    const CalendarDate& date,
    bool initial,
    const BondSnapshot& snapshot,
    const BondChangeSet& changes,
    char* output,
    std::size_t capacity
)
{
    if (!output || capacity == 0 ||
        snapshot.count > BOND_SLOT_COUNT)
        return false;

    output[0] = '\0';

    char date_text[64]{};
    if (!FormatCalendarDate(date, date_text, sizeof(date_text)))
        return false;

    std::size_t used = 0;
    const std::uint32_t current_sum = CalculateRateSum(snapshot);

    bool valid;

    if (initial) {
        valid = AppendFormat(
            output, capacity, used,
            "\"%s\",%lu,baseline,,%lu,,%lu,",
            date_text,
            static_cast<unsigned long>(date.elapsed_days),
            static_cast<unsigned long>(snapshot.count),
            static_cast<unsigned long>(current_sum)
        );
    } else {
        valid = AppendFormat(
            output, capacity, used,
            "\"%s\",%lu,%s,%lu,%lu,%lu,%lu,",
            date_text,
            static_cast<unsigned long>(date.elapsed_days),
            GetEventName(initial, changes),
            static_cast<unsigned long>(changes.previous_count),
            static_cast<unsigned long>(snapshot.count),
            static_cast<unsigned long>(changes.previous_rate_sum),
            static_cast<unsigned long>(current_sum)
        );
    }

    if (!valid || !AppendFormat(output, capacity, used, "\""))
        return false;

    for (std::size_t i = 0; i < snapshot.count; ++i) {
        if (!AppendFormat(
                output, capacity, used,
                "%s%u",
                i == 0 ? "" : ";",
                static_cast<unsigned int>(snapshot.rates[i])))
            return false;
    }

    if (!AppendFormat(output, capacity, used, "\",\""))
        return false;

    if (!initial) {
        bool first = true;

        for (std::size_t i = 0; i < changes.rate_slots.size(); ++i) {
            if (!changes.RateSlotChanged(i))
                continue;

            if (!AppendFormat(
                    output, capacity, used,
                    "%s%lu:%u->%u",
                    first ? "" : ";",
                    static_cast<unsigned long>(i),
                    static_cast<unsigned int>(changes.previous_slot_rates[i]),
                    static_cast<unsigned int>(changes.current_slot_rates[i])))
                return false;

            first = false;
        }
    }

    return AppendFormat(output, capacity, used, "\"\r\n");
}

}
