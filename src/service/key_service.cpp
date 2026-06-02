
#include "key_service.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "crypto.h"


KeyService::KeyService(
        IAuthRepository& auth_repo
) : m_auth_repo(auth_repo)
{ }

auto KeyService::create_key(
        const std::string& name,
        const std::vector<Permissions>& permissions,
        const std::string& expires_at
) -> CreateKeyResult {
    std::string raw_key = sologs::crypto::generate_key();
    std::string key_hash = sologs::crypto::sha256_hex(raw_key);

    AuthorizationEntry entry{
        .uuid = sologs::crypto::generate_uuid(),
        .key_hash = key_hash,
        .name = name,
        .permissions = permissions,
        .created_at = current_timestamp(),
        .expires_at = expires_at,
        .is_valid = true
    };

    m_auth_repo.insert(entry);

    return {raw_key, entry};
}

auto KeyService::current_timestamp() -> std::string {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

