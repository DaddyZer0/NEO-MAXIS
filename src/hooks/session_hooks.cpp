#include <windows.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "sc2kfix.h"
#include "neomaxis/city_session.h"

namespace {

neomaxis::CitySessionManager session_manager;
bool active_session_trustworthy = false;
std::string pending_load_path;

struct FileSnapshot {
    bool valid = false;
    bool exists = false;
    unsigned long long size = 0;
    FILETIME last_write{};
};

struct PendingSave {
    bool active = false;
    std::string path;
    std::string requested_name;
    bool change_city_name = false;
    FileSnapshot before;
};

PendingSave pending_save;

std::string FilenameStem(const char* path)
{
    if (path == nullptr || *path == '\0')
        return "Unnamed City";

    const char* base = path;
    const char* slash = std::strrchr(path, '\\');
    const char* forward_slash = std::strrchr(path, '/');

    if (slash != nullptr && slash + 1 > base)
        base = slash + 1;

    if (forward_slash != nullptr && forward_slash + 1 > base)
        base = forward_slash + 1;

    std::string result(base);
    const std::string::size_type dot = result.find_last_of('.');
    if (dot != std::string::npos)
        result.resize(dot);

    return result.empty() ? "Unnamed City" : result;
}

bool CaptureFileSnapshot(const char* path, FileSnapshot& snapshot)
{
    snapshot = {};

    if (path == nullptr || *path == '\0')
        return false;

    WIN32_FILE_ATTRIBUTE_DATA data{};

    if (!GetFileAttributesExA(
            path,
            GetFileExInfoStandard,
            &data)) {
        const DWORD error = GetLastError();

        if (error == ERROR_FILE_NOT_FOUND ||
            error == ERROR_PATH_NOT_FOUND) {
            snapshot.valid = true;
            snapshot.exists = false;
            return true;
        }

        return false;
    }

    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return false;

    ULARGE_INTEGER size{};
    size.HighPart = data.nFileSizeHigh;
    size.LowPart = data.nFileSizeLow;

    snapshot.valid = true;
    snapshot.exists = true;
    snapshot.size = size.QuadPart;
    snapshot.last_write = data.ftLastWriteTime;

    return true;
}

bool VerifySave(
    const FileSnapshot& before,
    const FileSnapshot& after)
{
    if (!before.valid || !after.valid ||
        !after.exists || after.size == 0)
        return false;

    // A newly created, nonempty save file is evidence of a save.
    if (!before.exists)
        return true;

    // An existing file must show a measurable change.
    return before.size != after.size ||
        CompareFileTime(
            &before.last_write,
            &after.last_write) != 0;
}

void AppendLifecycleEvent(
    const char* event,
    const char* filename,
    neomaxis::CitySessionId session_id = 0,
    const char* outcome = "not_applicable")
{
    if (event == nullptr)
        return;

    char message[1536]{};

    const int length = std::snprintf(
        message,
        sizeof(message),
        "NEO-MAXIS: Session lifecycle | Event=%s | File=%s | "
        "SessionId=%llu | Outcome=%s | SessionCount=%llu\r\n",
        event,
        filename != nullptr ? filename : "(none)",
        static_cast<unsigned long long>(session_id),
        outcome != nullptr ? outcome : "unknown",
        static_cast<unsigned long long>(
            session_manager.SessionCount())
    );

    if (length <= 0 ||
        static_cast<size_t>(length) >= sizeof(message))
        return;

    char module_path[MAX_PATH]{};
    char log_path[MAX_PATH * 2]{};

    const DWORD module_length = GetModuleFileNameA(
        nullptr,
        module_path,
        MAX_PATH
    );

    bool have_absolute_path = false;

    if (module_length > 0 && module_length < MAX_PATH) {
        char* slash = std::strrchr(module_path, '\\');
        char* forward_slash = std::strrchr(module_path, '/');

        if (forward_slash != nullptr &&
            (slash == nullptr || forward_slash > slash))
            slash = forward_slash;

        if (slash != nullptr) {
            *slash = '\0';

            const int path_length = std::snprintf(
                log_path,
                sizeof(log_path),
                "%s\\neomaxis-session.log",
                module_path
            );

            have_absolute_path =
                path_length > 0 &&
                static_cast<size_t>(path_length) < sizeof(log_path);
        }
    }

    if (!have_absolute_path)
        std::strcpy(log_path, "neomaxis-session.log");

    HANDLE file = CreateFileA(
        log_path,
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE)
        return;

    DWORD written = 0;

    WriteFile(
        file,
        message,
        static_cast<DWORD>(length),
        &written,
        nullptr
    );

    CloseHandle(file);
}

} // namespace

HOOKCB void Hook_OnNewCity_Before(void)
{
    const neomaxis::CitySessionId id =
        session_manager.BeginNewCity("Unnamed City");

    active_session_trustworthy = (id != 0);
    pending_load_path.clear();

    AppendLifecycleEvent(
        "new_city_session_created",
        nullptr,
        id,
        id != 0 ? "created" : "creation_failed"
    );
}

HOOKCB void L_SimcityApp_DoLoad_Before(
    void* app,
    char* filename)
{
    (void)app;

    pending_load_path =
        filename != nullptr ? filename : "";

    const bool started =
        !pending_load_path.empty() &&
        session_manager.BeginLoad(pending_load_path);

    AppendLifecycleEvent(
        "load_started",
        filename,
        session_manager.ActiveSession() != nullptr
            ? session_manager.ActiveSession()->id : 0,
        started ? "pending" : "could_not_start"
    );
}

HOOKCB void L_SimcityApp_DoLoad_After(
    void* app,
    char* filename)
{
    (void)app;

    // The actual result arrives through L_SimcityApp_DoLoad_Result.
    // Preserve the pending path and manager state until then.
    const char* path = !pending_load_path.empty()
        ? pending_load_path.c_str()
        : filename;

    AppendLifecycleEvent(
        "load_callback_completed",
        path,
        session_manager.ActiveSession() != nullptr
            ? session_manager.ActiveSession()->id : 0,
        session_manager.HasPendingLoad()
            ? "awaiting_result"
            : "no_pending_load"
    );
}

HOOKCB void L_SimcityApp_DoLoad_Result(
    void* app,
    char* filename,
    int result)
{
    (void)app;

    const bool succeeded = (result != 0);
    const bool had_pending_load = session_manager.HasPendingLoad();

    const std::string path = !pending_load_path.empty()
        ? pending_load_path
        : (filename != nullptr ? filename : "");

    bool session_updated = false;

    if (had_pending_load) {
        session_updated = session_manager.CompleteLoad(succeeded);
    }

    if (succeeded) {
        active_session_trustworthy =
            session_updated && session_manager.ActiveSession() != nullptr;
    }
    // On failure, CompleteLoad(false) preserves the previous active session
    // and its existing trust state.

    const char* outcome = "no_pending_load";

    if (had_pending_load) {
        if (!session_updated)
            outcome = "session_update_failed";
        else
            outcome = succeeded ? "load_succeeded" : "load_failed";
    }

    const auto* active = session_manager.ActiveSession();

    AppendLifecycleEvent(
        "load_result_received",
        path.empty() ? nullptr : path.c_str(),
        active != nullptr ? active->id : 0,
        outcome
    );

    pending_load_path.clear();
}

HOOKCB void L_SimcityApp_DoSave_Before(
    void* app,
    const char* filename,
    char* new_city_name,
    bool change_city_name)
{
    (void)app;

    pending_save = {};
    pending_save.active = filename != nullptr && *filename != '\0';
    pending_save.path = filename != nullptr ? filename : "";
    pending_save.change_city_name = change_city_name;

    if (change_city_name &&
        new_city_name != nullptr &&
        *new_city_name != '\0') {
        pending_save.requested_name = new_city_name;
    }

    if (pending_save.active) {
        CaptureFileSnapshot(
            pending_save.path.c_str(),
            pending_save.before
        );
    }

    const neomaxis::CitySession* active =
        session_manager.ActiveSession();

    AppendLifecycleEvent(
        "save_started",
        filename,
        active != nullptr ? active->id : 0,
        pending_save.before.valid
            ? (pending_save.before.exists
                ? "destination_existed"
                : "destination_new")
            : "destination_metadata_unavailable"
    );
}

HOOKCB void L_SimcityApp_DoSave_After(
    void* app,
    const char* filename,
    char* new_city_name,
    bool change_city_name)
{
    (void)app;
    (void)new_city_name;
    (void)change_city_name;

    const char* path = filename != nullptr
        ? filename
        : pending_save.path.c_str();

    FileSnapshot after{};
    const bool after_valid =
        CaptureFileSnapshot(path, after);

    const bool matching_operation =
        pending_save.active &&
        filename != nullptr &&
        pending_save.path == filename;

    const bool verified =
        matching_operation &&
        after_valid &&
        VerifySave(pending_save.before, after);

    if (!verified) {
        const neomaxis::CitySession* active =
            session_manager.ActiveSession();

        AppendLifecycleEvent(
            "save_callback_completed",
            path,
            active != nullptr ? active->id : 0,
            "outcome_unverified"
        );

        pending_save = {};
        return;
    }

    std::string display_name;

    if (!pending_save.requested_name.empty()) {
        display_name = pending_save.requested_name;
    } else if (active_session_trustworthy) {
        const neomaxis::CitySession* active =
            session_manager.ActiveSession();

        if (active != nullptr)
            display_name = active->display_name;
    }

    if (display_name.empty()) {
        const neomaxis::CitySession* existing =
            session_manager.FindBySavePath(path);

        if (existing != nullptr)
            display_name = existing->display_name;
    }

    if (display_name.empty())
        display_name = FilenameStem(path);

    neomaxis::CitySessionId session_id = 0;

    if (active_session_trustworthy) {
        const neomaxis::CitySession* active =
            session_manager.ActiveSession();

        if (active != nullptr)
            session_id = active->id;
    } else {
        const neomaxis::CitySession* existing =
            session_manager.FindBySavePath(path);

        if (existing != nullptr) {
            session_id = existing->id;
            session_manager.SetActiveSession(session_id);
        }
    }

    if (session_id == 0) {
        session_id = session_manager.BeginNewCity(display_name);
    }

    const bool associated =
        session_id != 0 &&
        session_manager.AssociateSavePath(
            session_id,
            path,
            display_name
        );

    if (associated)
        active_session_trustworthy = true;

    AppendLifecycleEvent(
        associated
            ? "save_verified_session_associated"
            : "save_verified_session_association_failed",
        path,
        session_id,
        associated ? "file_change_verified" : "association_failed"
    );

    pending_save = {};
}
