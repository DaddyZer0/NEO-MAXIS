#pragma once

#include <cstddef>
#include <cstdint>

#include "neomaxis/game_state.h"

namespace neomaxis {

struct CalendarDate {
    std::uint32_t year = 0;
    std::uint16_t month = 0;
    std::uint16_t day = 0;
    std::uint32_t elapsed_days = 0;
    std::uint32_t elapsed_years = 0;
};

CalendarDate CalculateCalendarDate(const GameState& state);

bool FormatCalendarDate(
    const CalendarDate& date,
    char* output,
    std::size_t output_size
);

}
