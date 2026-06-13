
#pragma once

#include <optional>

#include "auth.h"
#include "auth_repository.h"


class IAuthorizer {
    public:
        virtual ~IAuthorizer() = default;

        [[nodiscard]] virtual auto has_permissions(
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
        ) const -> bool = 0;

};

class Authorizer : public IAuthorizer {
    public:
        Authorizer();

        [[nodiscard]] auto has_permissions(
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
        ) const -> bool override;
};

class IAuthenticator {
    public:
        virtual ~IAuthenticator() = default;

        [[nodiscard]] virtual auto authenticate(
                const std::string& key
        ) const -> std::optional<Subject> = 0;

};

class Authenticator : public IAuthenticator {
    public:
        Authenticator(IAuthRepository& auth_repo);

        [[nodiscard]] auto authenticate(
                const std::string& key
        ) const -> std::optional<Subject> override;

    private:
        IAuthRepository& auth_repo_;
};

class IAuthService {
    public:
        struct CreateUserResult {
            std::string raw_key;
            AuthorizationEntry entry;
        };

        ~IAuthService() = default;

        [[nodiscard]] virtual auto create_user(
                const std::string& name,
                const std::vector<Permissions>& permissions,
                const std::string& expires_at
        ) -> CreateUserResult = 0;

        [[nodiscard]] virtual auto authenticate(
                const std::string& key
        ) const -> std::optional<Subject> = 0;

        [[nodiscard]] virtual auto subject_has_permissions(
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
        ) const -> bool = 0;

};

class AuthService : public IAuthService {
    public:
        explicit AuthService(IAuthRepository& auth_repo);
        
        [[nodiscard]] auto create_user(
                const std::string& name,
                const std::vector<Permissions>& permissions,
                const std::string& expires_at
        ) -> CreateUserResult override;

        [[nodiscard]] auto authenticate(
                const std::string& key
        ) const -> std::optional<Subject> override;

        [[nodiscard]] auto subject_has_permissions(
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
        ) const -> bool override;

    private:
        [[nodiscard]] auto current_timestamp() const noexcept -> std::string;

        IAuthRepository& auth_repo_;

};

