
#pragma once

#include <string>
#include <vector>


enum class Permissions {
    LogRead,
    LogWrite,
    LogDelete,
    AuthRead,
    AuthWrite,
    AuthDelete,
    Admin
};

enum class PermissionMode {
    AnyOf,
    AllOf
};

struct AuthorizationEntry {
    std::string uuid;
    std::string key_hash;
    std::string name;
    std::vector<Permissions> permissions;
    std::string created_at;
    std::string expires_at;
    bool is_valid;
};

struct Subject {
    std::string uuid;
    std::string name;
    std::vector<Permissions> permissions;
};

