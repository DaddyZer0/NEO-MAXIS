#include "neomaxis/budget.h"

#include <cstring>

namespace neomaxis {

bool TryDecodeBudgetSnapshot(
    const void* source,
    std::size_t source_size,
    BudgetSnapshot& snapshot
)
{
    if (source == nullptr)
        return false;

    if (source_size != sizeof(BudgetSnapshot))
        return false;

    BudgetSnapshot result{};

    std::memcpy(
        &result,
        source,
        sizeof(result)
    );

    snapshot = result;

    return true;
}

}
