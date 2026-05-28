
#pragma once

#include <optional>

#include "auth.h"
#include "auth_repository.h"


class IAuthorizer {
    public:
        virtual ~IAuthorizer() = default;

        virtual bool has_permissions(
            const Subject& subject,
            const std::vector<Permissions>& valid_permissions,
            PermissionMode mode
        ) const = 0;

};

class Authorizer : public IAuthorizer {
    public:
        Authorizer();

        bool has_permissions(
            const Subject& subject,
            const std::vector<Permissions>& valid_permissions,
            PermissionMode mode
        ) const override;
};

class IAuthenticator {
    public:
        virtual ~IAuthenticator() = default;
        virtual std::optional<Subject> authenticate(
            const std::string& key
        ) const = 0;
};

class Authenticator : public IAuthenticator {
    public:
        Authenticator(IAuthRepository& auth_repo);

        std::optional<Subject> authenticate(
            const std::string& key
        ) const override;

    private:
        IAuthRepository& m_auth_repo;
};


