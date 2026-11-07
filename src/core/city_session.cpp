#include "neomaxis/city_session.h"

#include <utility>

namespace neomaxis {

CitySessionId CitySessionManager::CreateSession(
    const std::string& display_name
)
{
    if (next_session_id_ == 0)
        return 0;

    CitySession session;
    session.id = next_session_id_++;
    session.display_name = display_name;

    sessions_.push_back(std::move(session));
    active_session_id_ = sessions_.back().id;

    return active_session_id_;
}

CitySessionId CitySessionManager::BeginNewCity(
    const std::string& display_name
)
{
    pending_load_path_.clear();
    return CreateSession(display_name);
}

bool CitySessionManager::BeginLoad(
    const std::string& save_path
)
{
    if (save_path.empty())
        return false;

    pending_load_path_ = save_path;
    return true;
}

bool CitySessionManager::CompleteLoad(
    bool succeeded,
    const std::string& display_name
)
{
    if (pending_load_path_.empty())
        return false;

    const std::string path = pending_load_path_;
    pending_load_path_.clear();

    if (!succeeded)
        return false;

    CitySession* existing = FindMutableBySavePath(path);

    if (existing != nullptr) {
        if (!display_name.empty())
            existing->display_name = display_name;

        active_session_id_ = existing->id;
        return true;
    }

    const CitySessionId id = CreateSession(display_name);

    if (id == 0)
        return false;

    sessions_.back().save_path = path;
    return true;
}

bool CitySessionManager::CancelPendingLoad()
{
    if (pending_load_path_.empty())
        return false;

    pending_load_path_.clear();
    return true;
}

bool CitySessionManager::AssociateSavePath(
    CitySessionId id,
    const std::string& save_path,
    const std::string& display_name
)
{
    if (save_path.empty())
        return false;

    CitySession* session = FindMutableById(id);

    if (session == nullptr)
        return false;

    CitySession* existing = FindMutableBySavePath(save_path);

    if (existing != nullptr && existing->id != id)
        return false;

    session->save_path = save_path;

    if (!display_name.empty())
        session->display_name = display_name;

    return true;
}

bool CitySessionManager::SetActiveSession(CitySessionId id)
{
    if (FindById(id) == nullptr)
        return false;

    active_session_id_ = id;
    return true;
}

const CitySession* CitySessionManager::ActiveSession() const
{
    return FindById(active_session_id_);
}

const CitySession* CitySessionManager::FindById(
    CitySessionId id
) const
{
    if (id == 0)
        return nullptr;

    for (const CitySession& session : sessions_) {
        if (session.id == id)
            return &session;
    }

    return nullptr;
}

const CitySession* CitySessionManager::FindBySavePath(
    const std::string& save_path
) const
{
    if (save_path.empty())
        return nullptr;

    for (const CitySession& session : sessions_) {
        if (session.save_path == save_path)
            return &session;
    }

    return nullptr;
}

CitySession* CitySessionManager::FindMutableById(
    CitySessionId id
)
{
    for (CitySession& session : sessions_) {
        if (session.id == id)
            return &session;
    }

    return nullptr;
}

CitySession* CitySessionManager::FindMutableBySavePath(
    const std::string& save_path
)
{
    for (CitySession& session : sessions_) {
        if (session.save_path == save_path)
            return &session;
    }

    return nullptr;
}

std::size_t CitySessionManager::SessionCount() const
{
    return sessions_.size();
}

bool CitySessionManager::HasPendingLoad() const
{
    return !pending_load_path_.empty();
}

void CitySessionManager::Reset()
{
    sessions_.clear();
    active_session_id_ = 0;
    pending_load_path_.clear();

    // Keep next_session_id_ monotonic for this manager's lifetime.
}

} // namespace neomaxis
