
#include "auth_service.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "crypto.h"
#include "utils.h"


UserLRUCache::UserLRUCache(int capacity) : capacity_(capacity) { }

auto UserLRUCache::get(
        const std::string& key
) noexcept -> std::optional<User> {
    std::lock_guard<std::mutex> _(mutex_);

    auto iter = cache_.find(key);
    if (iter == cache_.end()) {
        return std::nullopt;
    }

    lru_.splice(lru_.begin(), lru_, iter->second);
    return iter->second->second;
}

auto UserLRUCache::put(
    const std::string& key,
    User subject
) noexcept -> void {
    std::lock_guard<std::mutex> _(mutex_);

    auto iter = cache_.find(key);
    if (iter != cache_.end()) {
        iter->second->second = subject;
        lru_.splice(lru_.begin(), lru_, iter->second);
    }

    if (cache_.size() == capacity_) {
        std::string old_key = lru_.back().first;
        cache_.erase(old_key);
        lru_.pop_back();
    }

    lru_.emplace_front(key, subject);
    cache_[key] = lru_.begin();
}

AuthService::AuthService(
        IAuthRepository& auth_repo
) : auth_repo_(auth_repo),
    user_auth_cache_(32)
{ }

auto AuthService::create_user(
        const std::string& name,
        const PermissionList& permissions,
        const std::string& expires_at
) -> CreateUserResult {
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

    auth_repo_.insert(entry);

    return {raw_key, entry};
}

auto AuthService::get_users(
        const UserFilterParams& params
) -> json {
    auto validate = [](const std::optional<std::string>& field, const char* name) {
        if (field.has_value() && !sologs::utils::is_valid_datetime(field.value())) {
            throw std::invalid_argument(
                std::string(name) + " must be in YYYY-MM-DDTHH:MM:SS format"
            );
        }
    };
    validate(params.created_after, "created_after");
    validate(params.created_before, "created_before");
    validate(params.expires_after, "expires_after");
    validate(params.expires_before, "expires_before");

    UserFilterParams normalized = params;
    auto normalize = [](std::optional<std::string>& field) {
        if (field.has_value()) {
            if (field->back() == 'Z') field->pop_back();
            if ((*field)[10] == 'T') (*field)[10] = ' ';
        }
    };
    normalize(normalized.created_after);
    normalize(normalized.created_before);
    normalize(normalized.expires_after);
    normalize(normalized.expires_before);

    std::vector<AuthorizationEntry> users = auth_repo_.get_auth_entries(normalized);
    json arr = json::array();

    for (auto& user : users) {
        arr.push_back(
            {
                {"uuid", user.uuid}, 
                {"name", user.name}, 
                {"permissions", sologs::utils::permissions_to_string(user.permissions)}, 
                {"created_at", user.created_at},
                {"expires_at", user.expires_at},
                {"is_valid", user.is_valid ? "true" : "false"},
            }
        );
    }

    return arr;
}

auto AuthService::authenticate(
        const std::string& key
) -> std::optional<User> {
    if (auto subject = user_auth_cache_.get(key); subject.has_value()) {
        return subject.value();
    }

    auto hash = sologs::crypto::sha256_hex(key);
    auto entry = auth_repo_.get_by_key_hash(hash);
    if (!entry.has_value() || !entry->is_valid) {
        return {};
    }

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%d %H:%M:%S");

    if (entry->expires_at < ss.str()) {
        return {};
    }

    User subject{entry->uuid, entry->name, entry->permissions};
    user_auth_cache_.put(key, subject);

    return subject;
}

auto AuthService::subject_has_permissions(
        const User& subject,
        const PermissionList& valid_permissions,
        PermissionMode mode
) const -> bool {
    auto& subj_perms = subject.permissions;

    if (valid_permissions.empty()) {
        return false;
    }

    if (mode == PermissionMode::AllOf) {
        for (const auto& req_perm : valid_permissions) {
            if (std::find(subj_perms.begin(), subj_perms.end(), req_perm) == subj_perms.end()) {
                return false;
            }
        }
        return true; 
    }

    for (const auto& req_perm: valid_permissions) {
        auto iter = std::find(subj_perms.begin(), subj_perms.end(), req_perm);
        if (iter != subj_perms.end()) {
            return true;
        }
    }

    return false;
}

auto AuthService::current_timestamp() const noexcept -> std::string {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

