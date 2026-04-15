
#pragma once

#include <string>
#include <vector>

#include <sqlite3.h>


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

        sqlite3* get();

        bool execute(const std::string& query);

        bool execute_prepared(
                const std::string& query,
                const Row& values
        );

        bool execute_prepared_batched(
                const std::string& query,
                const std::vector<Row>& rows
        );

        QueryResult query(const std::string& query);

    private:
        sqlite3* m_db;

};


