#include "neomaxis/bond.h"

#include <array>
#include <cstdint>
#include <iostream>

namespace {

int checks = 0;
int failures = 0;

void Check(bool condition, const char* name)
{
    ++checks;

    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << name << '\n';
    }
}

void TestCountValidation()
{
    Check(neomaxis::IsValidBondCount(0), "zero bonds accepted");
    Check(neomaxis::IsValidBondCount(50), "maximum count accepted");
    Check(!neomaxis::IsValidBondCount(-1), "negative count rejected");
    Check(!neomaxis::IsValidBondCount(51), "excessive count rejected");
}

void TestEmptySnapshot()
{
    neomaxis::BondSnapshot snapshot{};

    Check(
        neomaxis::TryDecodeBondSnapshot(0, nullptr, 0, snapshot),
        "empty snapshot decoded"
    );

    Check(snapshot.count == 0, "empty snapshot count");
}

void TestFullSnapshot()
{
    std::array<std::uint16_t, 50> rates{};
    rates[0] = 4;
    rates[24] = 125;
    rates[49] = 65535;

    neomaxis::BondSnapshot snapshot{};

    Check(
        neomaxis::TryDecodeBondSnapshot(
            50, rates.data(), sizeof(rates), snapshot),
        "full snapshot decoded"
    );

    Check(snapshot.count == 50, "full snapshot count");
    Check(snapshot.rates[0] == 4, "first rate preserved");
    Check(snapshot.rates[24] == 125, "middle rate preserved");
    Check(snapshot.rates[49] == 65535, "last rate preserved");
}

void TestPartialSnapshot()
{
    const std::array<std::uint16_t, 3> rates = { 4, 7, 12 };
    neomaxis::BondSnapshot snapshot{};

    Check(
        neomaxis::TryDecodeBondSnapshot(
            3, rates.data(), sizeof(rates), snapshot),
        "partial snapshot decoded"
    );

    Check(snapshot.count == 3, "partial count preserved");
    Check(snapshot.rates[0] == 4, "partial first rate");
    Check(snapshot.rates[1] == 7, "partial second rate");
    Check(snapshot.rates[2] == 12, "partial third rate");
    Check(snapshot.rates[3] == 0, "unused slots zeroed");
    Check(snapshot.rates[49] == 0, "last unused slot zeroed");
}

void TestInvalidBuffers()
{
    const std::array<std::uint16_t, 3> rates = { 4, 7, 12 };
    neomaxis::BondSnapshot snapshot{};

    Check(
        !neomaxis::TryDecodeBondSnapshot(
            3, nullptr, sizeof(rates), snapshot),
        "null active source rejected"
    );

    Check(
        !neomaxis::TryDecodeBondSnapshot(
            3, rates.data(), sizeof(rates) - 1, snapshot),
        "undersized source rejected"
    );

    Check(
        !neomaxis::TryDecodeBondSnapshot(
            3, rates.data(), sizeof(rates) + 1, snapshot),
        "oversized source rejected"
    );

    Check(
        !neomaxis::TryDecodeBondSnapshot(
            51, rates.data(), sizeof(rates), snapshot),
        "invalid count rejected during decode"
    );
}

void TestFailurePreservesSnapshot()
{
    neomaxis::BondSnapshot snapshot{};
    snapshot.count = 1;
    snapshot.rates[0] = 99;

    const std::array<std::uint16_t, 2> rates = { 4, 7 };

    const bool result = neomaxis::TryDecodeBondSnapshot(
        3, rates.data(), sizeof(rates), snapshot
    );

    Check(!result, "invalid decode fails");
    Check(snapshot.count == 1, "previous count preserved");
    Check(snapshot.rates[0] == 99, "previous rate preserved");
}

}

int main()
{
    TestCountValidation();
    TestEmptySnapshot();
    TestFullSnapshot();
    TestPartialSnapshot();
    TestInvalidBuffers();
    TestFailurePreservesSnapshot();

    std::cout
        << "Bond Reader tests: "
        << checks << " checks, "
        << failures << " failures.\n";

    return failures == 0 ? 0 : 1;
}
