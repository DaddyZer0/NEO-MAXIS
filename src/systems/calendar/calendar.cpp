#include "neomaxis/calendar.h"

#include <stdio.h>

namespace neomaxis {

CalendarDate CalculateCalendarDate(const GameState& state)
{
    CalendarDate date{};

    date.elapsed_days = state.elapsed_days;
    date.elapsed_years = state.elapsed_days / 300;

    date.year = static_cast<std::uint32_t>(state.start_year)
        + date.elapsed_years;

    date.month = static_cast<std::uint16_t>(
        (state.elapsed_days / 25) % 12 + 1);

    date.day = static_cast<std::uint16_t>(
        (state.elapsed_days % 25) + 1);

    return date;
}

bool FormatCalendarDate(
    const CalendarDate& date,
    char* output,
    std::size_t output_size)
{
    static const char* month_names[] = {
        "January", "February", "March", "April",
        "May", "June", "July", "August",
        "September", "October", "November", "December"
    };

    if (output == nullptr || output_size == 0) {
        return false;
    }

    output[0] = '\0';

    if (date.month < 1 || date.month > 12 ||
        date.day < 1 || date.day > 25) {
        const int result = snprintf(
            output,
            output_size,
            "%s",
            "Invalid date"
        );

        return result >= 0 &&
            static_cast<std::size_t>(result) < output_size;
    }

    const int result = snprintf(
        output,
        output_size,
        "%s %u, %lu",
        month_names[date.month - 1],
        static_cast<unsigned int>(date.day),
        static_cast<unsigned long>(date.year)
    );

    return result >= 0 &&
        static_cast<std::size_t>(result) < output_size;
}

}
