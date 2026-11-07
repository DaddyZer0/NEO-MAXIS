#pragma once

#include <cstddef>
#include "neomaxis/calendar.h"
#include "neomaxis/bond.h"
#include "neomaxis/bond_diff.h"

namespace neomaxis {

bool FormatBondLedgerHeader(char* output, std::size_t capacity);

bool FormatBondLedgerRow(
    const CalendarDate& date,
    bool initial,
    const BondSnapshot& snapshot,
    const BondChangeSet& changes,
    char* output,
    std::size_t capacity
);

}
