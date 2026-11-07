#pragma once

#include <cstddef>
#include <cstdint>

namespace neomaxis {

struct MemoryRegionInfo {
    std::uintptr_t region_base = 0;
    std::size_t region_size = 0;
    std::uint32_t state = 0;
    std::uint32_t protection = 0;
    std::uint32_t type = 0;
};

bool TryReadGameMemory(
    std::uintptr_t rva,
    void* destination,
    std::size_t size
);

bool TryReadGamePointer(
    std::uintptr_t pointer_rva,
    std::uintptr_t& target
);


bool TryReadGameAddress(
    std::uintptr_t address,
    void* destination,
    std::size_t size
);

bool TryInspectReadableMemory(
    std::uintptr_t address,
    std::size_t size,
    MemoryRegionInfo& info
);

}
