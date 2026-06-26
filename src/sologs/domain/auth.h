
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

using PermissionList = std::vector<Permissions>;

struct AuthorizationEntry {
    std::string uuid;
    std::string key_hash;
    std::string name;
    PermissionList permissions;
    std::string created_at;
    std::string expires_at;
    bool is_valid;
};

struct User {
    std::string uuid;
    std::string name;
    PermissionList permissions;
};

struct UserFilterParams {
    static constexpr const char* UUIDKey            = "uuid";
    static constexpr const char* NameKey            = "name";
    static constexpr const char* PermissionsKey     = "permissions";
    static constexpr const char* CreatedAfterKey    = "created_after";
    static constexpr const char* CreatedBeforeKey   = "created_before";
    static constexpr const char* ExpiresAfterKey    = "expires_after";
    static constexpr const char* ExpiresBeforeKey   = "expires_before";
    static constexpr const char* IsValidKey         = "is_valid";
    static constexpr const char* LimitKey           = "limit";

    std::optional<std::string>      uuid{std::nullopt};
    std::optional<std::string>      name{std::nullopt};
    std::optional<PermissionList>   permissions{std::nullopt};
    std::optional<std::string>      created_after{std::nullopt};
    std::optional<std::string>      created_before{std::nullopt};
    std::optional<std::string>      expires_after{std::nullopt};
    std::optional<std::string>      expires_before{std::nullopt};
    std::optional<bool>             is_valid{std::nullopt};
    int                             limit{100};
};

