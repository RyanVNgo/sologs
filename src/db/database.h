
#pragma once

#include <string>
#include <vector>
#include <memory>


using Value = std::string;
using Row = std::vector<Value>;

using QueryResult = std::vector<Row>;

class SQLiteDatabase {
    public:
        SQLiteDatabase(const std::string& db_path);
        ~SQLiteDatabase();

        auto execute(const std::string& query) -> void;

        auto execute_prepared(
                const std::string& query,
                const Row& values
        ) -> void;

        auto execute_prepared_batched(
                const std::string& query,
                const std::vector<Row>& rows
        ) -> void;

        [[nodiscard]] auto query(const std::string& query) -> QueryResult;

        [[nodiscard]] auto query(
                const std::string& query,
                const Row& params
        ) -> QueryResult;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;

};


