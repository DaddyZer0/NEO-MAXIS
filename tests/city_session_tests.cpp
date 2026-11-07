#include "neomaxis/city_session.h"

#include <cstdlib>
#include <iostream>

namespace {

int checks = 0;
int failures = 0;

void Check(bool condition, const char* description)
{
    ++checks;

    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << description << '\n';
    }
}

} // namespace

int main()
{
    using namespace neomaxis;

    CitySessionManager manager;

    Check(manager.SessionCount() == 0,
        "manager starts without sessions");
    Check(manager.ActiveSession() == nullptr,
        "manager starts without active session");
    Check(!manager.BeginLoad(""),
        "empty load path rejected");

    const CitySessionId first =
        manager.BeginNewCity("New City");

    Check(first != 0,
        "first city receives an ID");
    Check(manager.ActiveSession() != nullptr,
        "new city becomes active");
    Check(manager.ActiveSession()->id == first,
        "active session ID matches first city");

    const CitySessionId second =
        manager.BeginNewCity("New City");

    Check(second != 0 && second != first,
        "duplicate city names receive distinct IDs");
    Check(manager.SessionCount() == 2,
        "both city sessions are retained");
    Check(manager.ActiveSession()->id == second,
        "new city replaces active session");

    Check(manager.AssociateSavePath(
        second, "C:/Cities/New City.sc2", "New City"),
        "save path associated with second city");

    Check(manager.FindBySavePath(
        "C:/Cities/New City.sc2")->id == second,
        "save path resolves to correct session");

    Check(manager.BeginLoad("C:/Cities/First.sc2"),
        "load transition begins");
    Check(manager.HasPendingLoad(),
        "pending load is recorded");

    Check(!manager.CompleteLoad(false),
        "failed load is rejected");
    Check(!manager.HasPendingLoad(),
        "failed load clears pending state");
    Check(manager.ActiveSession()->id == second,
        "failed load preserves active session");

    Check(manager.BeginLoad("C:/Cities/First.sc2"),
        "retry load begins");
    Check(manager.CompleteLoad(true, "First"),
        "successful load creates session");
    Check(manager.SessionCount() == 3,
        "loaded city receives its own session");
    Check(manager.ActiveSession()->display_name == "First",
        "loaded city becomes active");

    const CitySessionId first_loaded =
        manager.ActiveSession()->id;

    Check(manager.BeginLoad("C:/Cities/First.sc2"),
        "known city load begins");
    Check(manager.CompleteLoad(true),
        "known city load completes");
    Check(manager.ActiveSession()->id == first_loaded,
        "known save path reuses existing session ID");
    Check(manager.SessionCount() == 3,
        "reloading does not duplicate session");

    Check(manager.AssociateSavePath(
        first_loaded,
        "C:/Cities/Renamed City.sc2",
        "Renamed City"),
        "Save As updates path and display name");

    Check(manager.FindBySavePath(
        "C:/Cities/First.sc2") == nullptr,
        "old path no longer resolves after Save As");

    Check(manager.FindBySavePath(
        "C:/Cities/Renamed City.sc2")->id == first_loaded,
        "new path retains original session identity");

    Check(!manager.AssociateSavePath(
        second,
        "C:/Cities/Renamed City.sc2"),
        "path collision with another session rejected");

    Check(manager.SetActiveSession(second),
        "existing session can become active");
    Check(manager.ActiveSession()->id == second,
        "active session switches correctly");

    Check(!manager.SetActiveSession(999999),
        "unknown session cannot become active");

    Check(manager.BeginLoad("C:/Cities/Cancelled.sc2"),
        "cancel test load begins");
    Check(manager.CancelPendingLoad(),
        "pending load can be cancelled");
    Check(!manager.HasPendingLoad(),
        "cancel clears pending load");
    Check(manager.ActiveSession()->id == second,
        "cancel preserves active session");

    manager.Reset();

    Check(manager.SessionCount() == 0,
        "reset clears stored sessions");
    Check(manager.ActiveSession() == nullptr,
        "reset clears active session");

    const CitySessionId after_reset =
        manager.BeginNewCity("After Reset");

    Check(after_reset > first_loaded,
        "session IDs are not reused after reset");

    std::cout << "City session checks: " << checks
              << ", failures: " << failures << '\n';

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
