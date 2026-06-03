
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

        virtual ~IKeyService() = default;

        virtual auto create_key(
            const std::string& name,
            const std::vector<Permissions>& permissions,
            const std::string& expires_at
        ) -> CreateKeyResult = 0;

};

class KeyService: public IKeyService {
    public:
        explicit KeyService (IAuthRepository& auth_repo);

        auto create_key(
                const std::string& name,
                const std::vector<Permissions>& permissions,
                const std::string& expires_at
        ) -> CreateKeyResult override;

    private:
        static auto current_timestamp() -> std::string;

        IAuthRepository& auth_repo_;

};
