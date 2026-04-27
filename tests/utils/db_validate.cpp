
#include "db_utils.h"

#include <vector>
#include <algorithm>
#include <format>


bool util_table_exists(sqlite3* db, const char* table_name) {
    struct Tables {
        std::vector<std::string> names;
    };
    Tables tables;

    auto callback = [](void* out, int col_count, char** col_vals, char**) -> int {
        Tables* t = (Tables*)out;
        for (size_t i = 0; i < col_count; i++) {
            t->names.push_back(col_vals[i]);
        }
        return 0;
    };

    sqlite3_exec(
            db,
            "SELECT name FROM sqlite_schema WHERE type='table' AND name NOT LIKE 'sqlite_%';",
            callback,
            &tables,
            nullptr
    );

    auto target_iter = std::find(
            tables.names.begin(),
            tables.names.end(),
            table_name
    );
    if (target_iter == tables.names.end()) {
        return false;
    }

    return true;
}

bool util_validate_row_count(sqlite3* db, const char* table_name, int count) {
    std::vector<std::vector<std::string>> qr;
    auto callback = [](void* out, int col_count, char** col_vals, char**) -> int {
        std::vector<std::vector<std::string>>* t = (std::vector<std::vector<std::string>>*)out;
        std::vector<std::string> row;
        for (size_t i = 0; i < col_count; i++) {
            row.push_back(col_vals[i]);
        }
        t->push_back(row);
        return 0;
    };

    auto query = std::format(
            "SELECT COUNT(*) FROM {};",
            table_name
    );

    int rc = sqlite3_exec(
            db,
            query.c_str(),
            callback,
            &qr,
            nullptr
    );

    if (rc != SQLITE_OK) {
        return false;
    }

    if (qr.empty()) {
        return false;
    }

    int row_count = std::stoi(qr[0][0]);
    if (row_count != count) {
        return false;
    }

    return true;
}

bool util_validate_rows_exist(
        sqlite3* db, 
        const char* table_name, 
        const std::vector<std::vector<std::string>>& rows
) {
    std::vector<std::vector<std::string>> qr;
    auto callback = [](void* out, int col_count, char** col_vals, char**) -> int {
        std::vector<std::vector<std::string>>* t = (std::vector<std::vector<std::string>>*)out;
        std::vector<std::string> row;
        for (size_t i = 0; i < col_count; i++) {
            row.push_back(col_vals[i]);
        }
        t->push_back(row);
        return 0;
    };

    auto query = std::format(
            "SELECT * FROM {};",
            table_name
    );

    int rc = sqlite3_exec(
            db,
            query.c_str(),
            callback,
            &qr,
            nullptr
    );

    if (rc != SQLITE_OK) {
        return false;
    }

    if (qr.empty() && !rows.empty()) {
        return false;
    }

    for (const auto& row : rows) {
        auto iter = std::find(qr.begin(), qr.end(), row);
        if (iter == qr.end()) {
            return false;
        }
        qr.erase(iter);
    }

    return true;
}

