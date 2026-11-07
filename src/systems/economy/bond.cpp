#include "neomaxis/bond.h"

#include <cstring>

namespace neomaxis {

bool IsValidBondCount(std::int32_t count)
{
    return count >= 0 &&
        count <= static_cast<std::int32_t>(BOND_SLOT_COUNT);
}

bool TryDecodeBondSnapshot(
    std::int32_t count,
    const void* source,
    std::size_t source_size,
    BondSnapshot& snapshot
)
{
    if (!IsValidBondCount(count))
        return false;

    const std::size_t required_size =
        static_cast<std::size_t>(count) * sizeof(std::uint16_t);

    if (source_size != required_size)
        return false;

    if (required_size > 0 && source == nullptr)
        return false;

    BondSnapshot result{};
    result.count = static_cast<std::uint32_t>(count);

    if (required_size > 0) {
        std::memcpy(
            result.rates.data(),
            source,
            required_size
        );
    }

    snapshot = result;
    return true;
}

}
