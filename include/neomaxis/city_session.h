#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace neomaxis {

using CitySessionId = std::uint64_t;

struct CitySession {
    CitySessionId id = 0;
    std::string display_name;
    std::string save_path;
};

class CitySessionManager {
public:
    CitySessionId BeginNewCity(
        const std::string& display_name = {}
    );

    bool BeginLoad(const std::string& save_path);

    bool CompleteLoad(
        bool succeeded,
        const std::string& display_name = {}
    );

    bool CancelPendingLoad();

    bool AssociateSavePath(
        CitySessionId id,
        const std::string& save_path,
        const std::string& display_name = {}
    );

    bool SetActiveSession(CitySessionId id);

    const CitySession* ActiveSession() const;
    const CitySession* FindById(CitySessionId id) const;
    const CitySession* FindBySavePath(
        const std::string& save_path
    ) const;

    std::size_t SessionCount() const;

    bool HasPendingLoad() const;

    void Reset();

private:
    CitySessionId CreateSession(
        const std::string& display_name
    );

    CitySession* FindMutableById(CitySessionId id);
    CitySession* FindMutableBySavePath(
        const std::string& save_path
    );

    std::vector<CitySession> sessions_;
    CitySessionId active_session_id_ = 0;
    CitySessionId next_session_id_ = 1;
    std::string pending_load_path_;
};

} // namespace neomaxis
