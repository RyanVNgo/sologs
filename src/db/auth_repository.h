
#pragma once

#include <optional>

#include "database.h"
#include "auth.h"


class IAuthRepository {
    public:
        virtual ~IAuthRepository() = default; 

        virtual auto insert(const AuthorizationEntry& entry) -> bool = 0;

        virtual auto insert_batch(
                const std::vector<AuthorizationEntry>& entries
        ) -> bool = 0;

        virtual auto get_by_key_hash(
                const std::string& hash
        ) -> std::optional<AuthorizationEntry> = 0;

        virtual auto has_any_admin() -> bool = 0;

};

class SqlAuthRepository : public IAuthRepository {
    public:
        SqlAuthRepository(SQLiteDatabase& db);

        auto insert(const AuthorizationEntry& entry) -> bool override;

        auto insert_batch(
            const std::vector<AuthorizationEntry>& entries
        ) -> bool override;

        auto get_by_key_hash(
            const std::string& hash
        ) -> std::optional<AuthorizationEntry> override;

        auto has_any_admin() -> bool override;

    private:
        SQLiteDatabase& database_;

};


