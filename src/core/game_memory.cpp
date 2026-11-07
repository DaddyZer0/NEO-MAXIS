#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <limits>

#include "neomaxis/game_memory.h"

namespace neomaxis {

namespace {

constexpr std::uint32_t EXPECTED_IMAGE_SIZE = 0x00163000;

bool ValidateGameExecutable(HMODULE module)
{
    if (module == nullptr)
        return false;

    const std::uintptr_t base =
        reinterpret_cast<std::uintptr_t>(module);

    IMAGE_DOS_HEADER dos_header{};
    SIZE_T bytes_read = 0;

    if (!ReadProcessMemory(
            GetCurrentProcess(),
            reinterpret_cast<LPCVOID>(base),
            &dos_header,
            sizeof(dos_header),
            &bytes_read
        ) || bytes_read != sizeof(dos_header)) {
        return false;
    }

    if (dos_header.e_magic != IMAGE_DOS_SIGNATURE)
        return false;

    if (dos_header.e_lfanew <= 0 ||
        dos_header.e_lfanew > 0x100000) {
        return false;
    }

    IMAGE_NT_HEADERS32 nt_headers{};

    const std::uintptr_t nt_address =
        base + static_cast<std::uintptr_t>(dos_header.e_lfanew);

    bytes_read = 0;

    if (!ReadProcessMemory(
            GetCurrentProcess(),
            reinterpret_cast<LPCVOID>(nt_address),
            &nt_headers,
            sizeof(nt_headers),
            &bytes_read
        ) || bytes_read != sizeof(nt_headers)) {
        return false;
    }

    if (nt_headers.Signature != IMAGE_NT_SIGNATURE)
        return false;

    if (nt_headers.FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
        return false;

    if (nt_headers.OptionalHeader.Magic !=
        IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return false;
    }

    return nt_headers.OptionalHeader.SizeOfImage ==
           EXPECTED_IMAGE_SIZE;
}

bool IsReadableProtection(DWORD protection)
{
    if ((protection & PAGE_GUARD) != 0)
        return false;

    const DWORD basic_protection = protection & 0xFF;

    return basic_protection == PAGE_READONLY ||
           basic_protection == PAGE_READWRITE ||
           basic_protection == PAGE_WRITECOPY ||
           basic_protection == PAGE_EXECUTE_READ ||
           basic_protection == PAGE_EXECUTE_READWRITE ||
           basic_protection == PAGE_EXECUTE_WRITECOPY;
}

}

bool TryReadGameMemory(
    std::uintptr_t rva,
    void* destination,
    std::size_t size
)
{
    if (destination == nullptr || size == 0)
        return false;

    if (rva > EXPECTED_IMAGE_SIZE)
        return false;

    if (size > EXPECTED_IMAGE_SIZE - rva)
        return false;

    HMODULE game_module = GetModuleHandleA(nullptr);

    if (!ValidateGameExecutable(game_module))
        return false;

    const std::uintptr_t base =
        reinterpret_cast<std::uintptr_t>(game_module);

    SIZE_T bytes_read = 0;

    return ReadProcessMemory(
        GetCurrentProcess(),
        reinterpret_cast<LPCVOID>(base + rva),
        destination,
        size,
        &bytes_read
    ) != FALSE && bytes_read == size;
}

bool TryReadGamePointer(
    std::uintptr_t pointer_rva,
    std::uintptr_t& target
)
{
    std::uint32_t raw_pointer = 0;

    if (!TryReadGameMemory(
            pointer_rva,
            &raw_pointer,
            sizeof(raw_pointer)
        )) {
        return false;
    }

    target = static_cast<std::uintptr_t>(raw_pointer);

    return target != 0;
}


bool TryReadGameAddress(
    std::uintptr_t address,
    void* destination,
    std::size_t size
)
{
    if (destination == nullptr || size == 0)
        return false;

    MemoryRegionInfo region{};

    if (!TryInspectReadableMemory(address, size, region))
        return false;

    SIZE_T bytes_read = 0;

    return ReadProcessMemory(
        GetCurrentProcess(),
        reinterpret_cast<LPCVOID>(address),
        destination,
        size,
        &bytes_read
    ) != FALSE && bytes_read == size;
}

bool TryInspectReadableMemory(
    std::uintptr_t address,
    std::size_t size,
    MemoryRegionInfo& info
)
{
    if (address == 0 || size == 0)
        return false;

    if (size > (std::numeric_limits<std::uintptr_t>::max)() - address)
        return false;

    MEMORY_BASIC_INFORMATION memory{};

    if (VirtualQuery(
            reinterpret_cast<LPCVOID>(address),
            &memory,
            sizeof(memory)
        ) != sizeof(memory)) {
        return false;
    }

    if (memory.State != MEM_COMMIT)
        return false;

    if (!IsReadableProtection(memory.Protect))
        return false;

    const std::uintptr_t region_base =
        reinterpret_cast<std::uintptr_t>(memory.BaseAddress);

    if (memory.RegionSize >
        (std::numeric_limits<std::uintptr_t>::max)() - region_base) {
        return false;
    }

    const std::uintptr_t region_end =
        region_base + memory.RegionSize;

    const std::uintptr_t requested_end = address + size;

    if (address < region_base || requested_end > region_end)
        return false;

    MemoryRegionInfo result{};

    result.region_base = region_base;
    result.region_size = memory.RegionSize;
    result.state = memory.State;
    result.protection = memory.Protect;
    result.type = memory.Type;

    info = result;

    return true;
}

}
