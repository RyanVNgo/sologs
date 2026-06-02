
#pragma once

#include <string>
#include <vector>

#include "auth.h"
#include "auth_repository.h"


class IKeyService {
    public:
        struct CreateKeyResult {
            std::string raw_key;
            AuthorizationEntry entry;
        };

        virtual ~IKeyService() {};
        virtual CreateKeyResult create_key(
            const std::string& name,
            const std::vector<Permissions>& permissions,
            const std::string& expires_at
        ) = 0;
};

class KeyService: public IKeyService {
    public:
        explicit KeyService (IAuthRepository& auth_repo);

        CreateKeyResult create_key(
            const std::string& name,
            const std::vector<Permissions>& permissions,
            const std::string& expires_at
        ) override;

    private:
        IAuthRepository& m_auth_repo;

        static std::string current_timestamp();

};
