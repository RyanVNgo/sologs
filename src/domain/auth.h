
#pragma once

#include <string>
#include <vector>
#include <optional>


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

struct UserFilterParams {
    std::optional<std::string> uuid;
    std::optional<std::string> name;
    std::optional<std::vector<Permissions>> permissions;
    std::optional<std::string> created_after;
    std::optional<std::string> created_before;
    std::optional<std::string> expires_after;
    std::optional<std::string> expires_before;
    std::optional<bool> is_valid;
    int limit;
};

