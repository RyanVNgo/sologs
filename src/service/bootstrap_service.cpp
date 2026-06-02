
#include "bootstrap_service.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "crypto.h"
#include "auth.h"


auto BootstrapService::try_bootstrap(IAuthRepository& auth_repo) -> void {
    if (auth_repo.has_any_admin()) {
        return;
    }

    const char* raw_key = std::getenv("SOLOGS_BOOTSTRAP_KEY");
    if (!raw_key) {
        throw std::runtime_error(
            "No admins exist and SOLOGS_BOOTSTRAP_KEY environment variable is not set"
        );
    }

    std::string key_str(raw_key);
    std::string key_hash = sologs::crypto::sha256_hex(key_str);

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%d %H:%M:%S");

    AuthorizationEntry entry{
        .uuid = sologs::crypto::generate_uuid(),
        .key_hash = key_hash,
        .name = "bootstrap-admin",
        .permissions = {Permissions::Admin},
        .created_at = ss.str(),
        .expires_at = "9999-12-31 23:59:59",
        .is_valid = true
    };

    auth_repo.insert(entry);

    std::cout << "Bootstrapped admin key from SOLOGS_BOOTSTRAP_KEY" << std::endl;
    std::cout << "Also here's a random key: " << sologs::crypto::generate_key() << std::endl;
}

