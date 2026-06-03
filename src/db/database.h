
#pragma once

#include <string>
#include <vector>
#include <memory>


using Value = std::string;
using Row = std::vector<Value>;

using ColumnName = std::string;
using ColumnType = std::string;
struct Column {
    ColumnName name;
    ColumnType type;
};

using Schema = std::vector<Column>;
using Tables = std::vector<std::string>;

using QueryResult = std::vector<Row>;

class SQLiteDatabase {
    public:
        SQLiteDatabase(const std::string& db_path);
        ~SQLiteDatabase();

        auto execute(const std::string& query) -> bool;

        auto execute_prepared(
                const std::string& query,
                const Row& values
        ) -> bool;

        auto execute_prepared_batched(
                const std::string& query,
                const std::vector<Row>& rows
        ) -> bool;

        auto query(const std::string& query) -> QueryResult;

        auto query(
                const std::string& query,
                const Row& params
        ) -> QueryResult;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;

};


