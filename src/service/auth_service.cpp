
#include "auth_service.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <openssl/evp.h>
#include <sstream>


Authorizer::Authorizer() { }

bool Authorizer::has_permissions(
    const Subject& subject,
    const std::vector<Permissions>& valid_permissions,
    PermissionMode mode
) const {
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

static std::string sha256_hex(const std::string& input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, input.data(), input.size());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}


Authenticator::Authenticator(IAuthRepository& auth_repo) 
    :   m_auth_repo(auth_repo)
{ }

std::optional<Subject> Authenticator::authenticate(
    const std::string& key
) const {
    auto hash = sha256_hex(key);
    auto entry = m_auth_repo.get_by_key_hash(hash);
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

    return Subject{entry->uuid, entry->name, entry->permissions};
}


