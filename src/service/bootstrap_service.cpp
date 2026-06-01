
#include "bootstrap_service.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "sha256.h"
#include "auth.h"


void BootstrapService::try_bootstrap(IAuthRepository& auth_repo) {
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
    std::string key_hash = sha256_hex(key_str);

    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&now_c), "%Y-%m-%d %H:%M:%S");

    AuthorizationEntry entry;
    entry.uuid = key_hash.substr(0, 36);
    entry.key_hash = key_hash;
    entry.name = "bootstrap-admin";
    entry.permissions = {Permissions::Admin};
    entry.created_at = ss.str();
    entry.expires_at = "9999-12-31 23:59:59";
    entry.is_valid = true;

    auth_repo.insert(entry);

    std::cout << "Bootstrapped admin key from SOLOGS_BOOTSTRAP_KEY" << std::endl;
}

