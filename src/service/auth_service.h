
#pragma once

#include <optional>

#include "auth.h"
#include "auth_repository.h"


class IAuthorizer {
    public:
        virtual ~IAuthorizer() = default;

        virtual auto has_permissions(
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
        ) const -> bool = 0;

};

class Authorizer : public IAuthorizer {
    public:
        Authorizer();

        auto has_permissions(
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
        ) const -> bool override;
};

class IAuthenticator {
    public:
        virtual ~IAuthenticator() = default;

        virtual auto authenticate(
                const std::string& key
        ) const -> std::optional<Subject> = 0;

};

class Authenticator : public IAuthenticator {
    public:
        Authenticator(IAuthRepository& auth_repo);

        auto authenticate(
                const std::string& key
        ) const -> std::optional<Subject> override;

    private:
        IAuthRepository& auth_repo_;
};


