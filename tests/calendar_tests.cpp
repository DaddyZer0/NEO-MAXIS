#include "neomaxis/calendar.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int passed = 0;
int failed = 0;

void CheckDate(
    const std::string& name,
    std::uint32_t elapsed_days,
    std::uint16_t start_year,
    std::uint32_t expected_year,
    std::uint16_t expected_month,
    std::uint16_t expected_day,
    std::uint32_t expected_elapsed_years)
{
    neomaxis::GameState state{};
    state.elapsed_days = elapsed_days;
    state.start_year = start_year;

    const auto date = neomaxis::CalculateCalendarDate(state);

    const bool success =
        date.year == expected_year &&
        date.month == expected_month &&
        date.day == expected_day &&
        date.elapsed_days == elapsed_days &&
        date.elapsed_years == expected_elapsed_years;

    if (success) {
        ++passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++failed;
        std::cout << "[FAIL] " << name
                  << " | Expected: "
                  << expected_year << '/'
                  << expected_month << '/'
                  << expected_day
                  << " | Got: "
                  << date.year << '/'
                  << date.month << '/'
                  << date.day << '\n';
    }
}

void CheckFormattedDate(
    const std::string& name,
    const neomaxis::CalendarDate& date,
    const std::string& expected)
{
    char actual[64]{};

    const bool formatted =
        neomaxis::FormatCalendarDate(date, actual, sizeof(actual));

    const bool success = formatted && expected == actual;

    if (success) {
        ++passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++failed;
        std::cout << "[FAIL] " << name
                  << " | Expected: " << expected
                  << " | Got: "
                  << (formatted ? actual : "<format failed>")
                  << '\n';
    }
}

}

int main()
{
    std::cout << "NEO-MAXIS Calendar Test Suite\n\n";

    CheckDate("Initial date", 0, 1900, 1900, 1, 1, 0);
    CheckDate("Second day", 1, 1900, 1900, 1, 2, 0);
    CheckDate("Last day of January", 24, 1900, 1900, 1, 25, 0);

    for (std::uint32_t month = 1; month <= 12; ++month) {
        const std::uint32_t first_day = (month - 1) * 25;
        const std::uint32_t last_day = first_day + 24;

        CheckDate(
            "Month " + std::to_string(month) + " begins",
            first_day, 1900, 1900,
            static_cast<std::uint16_t>(month), 1, 0);

        CheckDate(
            "Month " + std::to_string(month) + " ends",
            last_day, 1900, 1900,
            static_cast<std::uint16_t>(month), 25, 0);
    }

    CheckDate("First day of year 1901", 300, 1900, 1901, 1, 1, 1);
    CheckDate("First day of year 1902", 600, 1900, 1902, 1, 1, 2);
    CheckDate("First day of year 1903", 900, 1900, 1903, 1, 1, 3);

    CheckDate("City starting in 1950", 0, 1950, 1950, 1, 1, 0);
    CheckDate("1950 city after one year", 300, 1950, 1951, 1, 1, 1);

    CheckDate("City starting in 2000", 0, 2000, 2000, 1, 1, 0);
    CheckDate("City starting in 2050", 0, 2050, 2050, 1, 1, 0);

    CheckFormattedDate(
        "Format January 1",
        {1900, 1, 1, 0, 0},
        "January 1, 1900");

    CheckFormattedDate(
        "Format February 25",
        {1900, 2, 25, 49, 0},
        "February 25, 1900");

    CheckFormattedDate(
        "Format December 25",
        {1900, 12, 25, 299, 0},
        "December 25, 1900");

    CheckFormattedDate(
        "Reject invalid month",
        {1900, 13, 1, 0, 0},
        "Invalid date");

    CheckFormattedDate(
        "Reject invalid day",
        {1900, 1, 26, 0, 0},
        "Invalid date");

    std::cout << "\n==============================\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total:  " << passed + failed << '\n';

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
