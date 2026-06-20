
#pragma once

#include <optional>

#include "database.h"
#include "auth.h"


class IAuthRepository {
    public:
        virtual ~IAuthRepository() = default; 

        virtual auto insert(const AuthorizationEntry& entry) -> void = 0;

        virtual auto insert_batch(
                const std::vector<AuthorizationEntry>& entries
        ) -> void = 0;

        [[nodiscard]]
        virtual auto get_auth_entries(
                const UserFilterParams& params
        ) -> std::vector<AuthorizationEntry> = 0;

        [[nodiscard]]
        virtual auto get_by_key_hash(
                const std::string& hash
        ) const -> std::optional<AuthorizationEntry> = 0;

        [[nodiscard]]
        virtual auto has_any_admin() const -> bool = 0;

};

class SqlAuthRepository : public IAuthRepository {
    public:
        SqlAuthRepository(SQLiteDatabase& db);

        auto insert(const AuthorizationEntry& entry) -> void override;

        auto insert_batch(
            const std::vector<AuthorizationEntry>& entries
        ) -> void override;

        [[nodiscard]]
        auto get_auth_entries(
                const UserFilterParams& params
        ) -> std::vector<AuthorizationEntry> override;

        [[nodiscard]]
        auto get_by_key_hash(
            const std::string& hash
        ) const -> std::optional<AuthorizationEntry> override;

        [[nodiscard]]
        auto has_any_admin() const -> bool override;

    private:
        SQLiteDatabase& database_;

};


