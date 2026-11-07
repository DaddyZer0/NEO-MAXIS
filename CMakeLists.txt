cmake_minimum_required(VERSION 3.20)

set(VERSION_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/include/neomaxis/version.h"
)

file(STRINGS "${VERSION_HEADER}" VERSION_MAJOR_LINE
    REGEX "^#define NEOMAXIS_VERSION_MAJOR [0-9]+$"
)
file(STRINGS "${VERSION_HEADER}" VERSION_MINOR_LINE
    REGEX "^#define NEOMAXIS_VERSION_MINOR [0-9]+$"
)
file(STRINGS "${VERSION_HEADER}" VERSION_PATCH_LINE
    REGEX "^#define NEOMAXIS_VERSION_PATCH [0-9]+$"
)

foreach(PART MAJOR MINOR PATCH)
    if(PART STREQUAL "MAJOR")
        set(VERSION_LINE "${VERSION_MAJOR_LINE}")
    elseif(PART STREQUAL "MINOR")
        set(VERSION_LINE "${VERSION_MINOR_LINE}")
    else()
        set(VERSION_LINE "${VERSION_PATCH_LINE}")
    endif()

    if(NOT VERSION_LINE)
        message(FATAL_ERROR
            "Missing NEO-MAXIS version component: ${PART}"
        )
    endif()

    string(REGEX REPLACE
        "^#define NEOMAXIS_VERSION_${PART} ([0-9]+)$"
        "\\1"
        VERSION_${PART}
        "${VERSION_LINE}"
    )
endforeach()

project(NEO_MAXIS
    VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NEOMAXIS_BUILD_TESTS
    "Build native NEO-MAXIS unit tests instead of the Windows DLL"
    OFF
)

function(neomaxis_configure_target target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
        )
    endif()
endfunction()

if(NEOMAXIS_BUILD_TESTS)

    enable_testing()

    add_executable(neomaxis_calendar_tests
        tests/calendar_tests.cpp
        src/systems/calendar/calendar.cpp
    )

    add_executable(neomaxis_economy_budget_tests
        tests/economy_budget_tests.cpp
        src/systems/economy/budget.cpp
    )

    add_executable(neomaxis_budget_diff_tests
        tests/budget_diff_tests.cpp
        src/systems/economy/budget_diff.cpp
    )

    add_executable(neomaxis_bond_reader_tests
        tests/bond_reader_tests.cpp
        src/systems/economy/bond.cpp
    )

    add_executable(neomaxis_bond_diff_tests
        tests/bond_diff_tests.cpp
        src/systems/economy/bond_diff.cpp
    )

    add_executable(neomaxis_bond_ledger_tests
        tests/bond_ledger_tests.cpp
        src/systems/economy/bond_ledger.cpp
        src/systems/economy/bond_diff.cpp
        src/systems/calendar/calendar.cpp
    )

    add_executable(neomaxis_bond_analytics_tests
        tests/bond_analytics_tests.cpp
        src/systems/economy/bond_analytics.cpp
        src/systems/economy/bond.cpp
    )

    add_executable(neomaxis_city_session_tests
        tests/city_session_tests.cpp
        src/core/city_session.cpp
    )

    set(NEOMAXIS_TEST_TARGETS
        neomaxis_calendar_tests
        neomaxis_economy_budget_tests
        neomaxis_budget_diff_tests
        neomaxis_bond_reader_tests
        neomaxis_bond_diff_tests
        neomaxis_bond_ledger_tests
        neomaxis_bond_analytics_tests
        neomaxis_city_session_tests
    )

    foreach(target IN LISTS NEOMAXIS_TEST_TARGETS)
        target_include_directories(${target} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
        )

        neomaxis_configure_target(${target})
    endforeach()

    add_test(
        NAME calendar_tests
        COMMAND neomaxis_calendar_tests
    )

    add_test(
        NAME economy_budget_tests
        COMMAND neomaxis_economy_budget_tests
    )

    add_test(
        NAME budget_diff_tests
        COMMAND neomaxis_budget_diff_tests
    )

    add_test(
        NAME bond_reader_tests
        COMMAND neomaxis_bond_reader_tests
    )

    add_test(
        NAME bond_diff_tests
        COMMAND neomaxis_bond_diff_tests
    )

    add_test(
        NAME bond_ledger_tests
        COMMAND neomaxis_bond_ledger_tests
    )

    add_test(
        NAME bond_analytics_tests
        COMMAND neomaxis_bond_analytics_tests
    )

    add_test(
        NAME city_session_tests
        COMMAND neomaxis_city_session_tests
    )

else()

    set(SC2KFIX_SOURCE_DIR "" CACHE PATH
        "Path to the sc2kfix source checkout"
    )

    if(NOT EXISTS "${SC2KFIX_SOURCE_DIR}/mods/sc2kfix.h")
        message(FATAL_ERROR
            "sc2kfix SDK header not found.\n"
            "Set SC2KFIX_SOURCE_DIR to the sc2kfix source directory.\n"
            "Example:\n"
            "  -DSC2KFIX_SOURCE_DIR=/path/to/sc2kfix-source"
        )
    endif()

    add_library(neomaxis SHARED
        src/mod_info.cpp
        src/core/game_memory.cpp
        src/core/game_state.cpp
        src/core/city_session.cpp
        src/systems/calendar/calendar.cpp
        src/systems/economy/economy_reader.cpp
        src/systems/economy/budget.cpp
        src/systems/economy/budget_reader.cpp
        src/systems/economy/bond.cpp
        src/systems/economy/bond_reader.cpp
        src/systems/economy/bond_diff.cpp
        src/systems/economy/bond_ledger.cpp
        src/systems/economy/bond_analytics.cpp
        src/systems/economy/budget_diff.cpp
        src/hooks/calendar_hooks.cpp
        src/hooks/session_hooks.cpp
    )

    target_include_directories(neomaxis PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${SC2KFIX_SOURCE_DIR}/mods"
    )

    neomaxis_configure_target(neomaxis)

    if(MSVC)
        target_compile_options(neomaxis PRIVATE
            "$<$<CONFIG:Release>:/O2>"
        )
    elseif(MINGW)
        target_compile_options(neomaxis PRIVATE
            "$<$<CONFIG:Release>:-O2>"
        )

        target_link_options(neomaxis PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
        )
    else()
        target_compile_options(neomaxis PRIVATE
            "$<$<CONFIG:Release>:-O2>"
        )
    endif()

    set_target_properties(neomaxis PROPERTIES
        PREFIX ""
        OUTPUT_NAME "neomaxis"
        SUFFIX ".dll"
    )

endif()
