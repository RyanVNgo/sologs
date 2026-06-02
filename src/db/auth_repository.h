
#pragma once

#include <optional>

#include "database.h"
#include "auth.h"


class IAuthRepository {
    public:
        virtual ~IAuthRepository() = default; 

        virtual bool insert(const AuthorizationEntry& entry) = 0;
        virtual bool insert_batch(
            const std::vector<AuthorizationEntry>& entries
        ) = 0;
        virtual std::optional<AuthorizationEntry> get_by_key_hash(
            const std::string& hash
        ) = 0;

        virtual bool has_any_admin() = 0;
    };

class SqlAuthRepository : public IAuthRepository {
    public:
        SqlAuthRepository(SQLiteDatabase& db);

        bool insert(const AuthorizationEntry& entry) override;
        bool insert_batch(
            const std::vector<AuthorizationEntry>& entries
        ) override;
        std::optional<AuthorizationEntry> get_by_key_hash(
            const std::string& hash
        ) override;

        bool has_any_admin() override;

    private:
        SQLiteDatabase& m_database;

};


