
#pragma once

#include <optional>
#include <list>
#include <unordered_map>
#include <mutex>

#include <nlohmann/json.hpp>

#include <domain/auth.h>
#include <database/auth_repository.h>


using json = nlohmann::json;

class IUserService {
    public:
        struct CreateUserResult {
            std::string raw_key;
            AuthorizationEntry entry;
        };

        ~IUserService() = default;

        [[nodiscard]]
        virtual auto create_user(
                const std::string& name,
                const PermissionList& permissions,
                const std::string& expires_at
        ) -> CreateUserResult = 0;

        [[nodiscard]]
        virtual auto get_users(
                const UserFilterParams& params
        ) -> json = 0;

        [[nodiscard]]
        virtual auto authenticate(
                const std::string& key
        ) -> std::optional<User> = 0;

        [[nodiscard]]
        virtual auto subject_has_permissions(
                const User& subject,
                const PermissionList& valid_permissions,
                PermissionMode mode
        ) const -> bool = 0;

};

class UserLRUCache {
    public:
        UserLRUCache(int capacity);

        [[nodiscard]]
        auto get(
                const std::string& key
        ) noexcept -> std::optional<User>;

        auto put(const std::string& key, User subject) noexcept -> void;

    private:
        using KeySubjectPair = std::pair<std::string, User>;

        size_t capacity_;
        std::list<KeySubjectPair> lru_;
        std::unordered_map<
            std::string,
            std::list<KeySubjectPair>::iterator
        > cache_;

        mutable std::mutex mutex_;

};

class UserService : public IUserService {
    public:
        explicit UserService(IAuthRepository& auth_repo);
        
        [[nodiscard]]
        auto create_user(
                const std::string& name,
                const PermissionList& permissions,
                const std::string& expires_at
        ) -> CreateUserResult override;

        [[nodiscard]]
        auto get_users(
                const UserFilterParams& params
        ) -> json override;

        [[nodiscard]]
        auto authenticate(
                const std::string& key
        ) -> std::optional<User> override;

        [[nodiscard]]
        auto subject_has_permissions(
                const User& subject,
                const PermissionList& valid_permissions,
                PermissionMode mode
        ) const -> bool override;

    private:
        [[nodiscard]]
        auto current_timestamp() const noexcept -> std::string;

        IAuthRepository& auth_repo_;
        UserLRUCache user_auth_cache_;

};

