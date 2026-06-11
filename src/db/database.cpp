
#include "database.h"

#include <iostream>

#include <sqlite3.h>


struct SQLiteDatabase::Impl {
    sqlite3* db;
};

SQLiteDatabase::SQLiteDatabase(
        const std::string& db_path
) : pimpl_(new Impl())
{
    if (sqlite3_open(db_path.c_str(), &(pimpl_->db)) != SQLITE_OK) {
        std::string msg = sqlite3_errmsg(pimpl_->db);
        sqlite3_close(pimpl_->db);
        throw std::runtime_error("Failed to open database: " + msg);
    }
}

SQLiteDatabase::~SQLiteDatabase() {
    if (pimpl_->db) {
        sqlite3_close(pimpl_->db);
    }
}

auto SQLiteDatabase::execute(const std::string& query) -> void {
    char* err_msg = nullptr;
    int rc = sqlite3_exec(
            pimpl_->db,
            query.c_str(),
            nullptr,
            nullptr,
            &err_msg
    ); 

    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error(err_str);
    } 
}

auto SQLiteDatabase::execute_prepared(
        const std::string& query,
        const Row& values
) -> void {
    sqlite3_stmt* stmt = nullptr;

    if (int err_code = sqlite3_prepare_v2(
                pimpl_->db,
                query.c_str(),
                -1,
                &stmt,
                nullptr
        ); err_code != SQLITE_OK
    ) {
        throw std::runtime_error(sqlite3_errstr(err_code));
    }

    for (size_t i = 0; i < values.size(); ++i) {
        if (int err_code = sqlite3_bind_text(
                    stmt,
                    static_cast<int>(i + 1),
                    values[i].c_str(),
                    -1,
                    SQLITE_TRANSIENT
            ); err_code != SQLITE_OK
        ) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(sqlite3_errstr(err_code));
        }
    }

    if (int err_code = sqlite3_step(stmt); err_code != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errstr(err_code));
    }

    sqlite3_finalize(stmt);
}

auto SQLiteDatabase::execute_prepared_batched(
        const std::string& query,
        const std::vector<Row>& rows
) -> void {
    sqlite3_stmt* stmt = nullptr;

    if (int err_code = sqlite3_prepare_v2(
                pimpl_->db,
                query.c_str(),
                -1,
                &stmt,
                nullptr
        ); err_code != SQLITE_OK
    ) {
        throw std::runtime_error(sqlite3_errstr(err_code));
    }

    try {
        execute("BEGIN TRANSACTION;");
        for (const auto& values : rows) {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);

            for (int i = 0; i < values.size(); ++i) {
                if (int err_code = sqlite3_bind_text(
                        stmt,
                        i + 1,
                        values[i].c_str(),
                        -1,
                        SQLITE_STATIC
                    ); err_code != SQLITE_OK
                ) {
                    throw std::runtime_error(sqlite3_errstr(err_code));
                }
            }

            if (int err_code = sqlite3_step(stmt); err_code != SQLITE_DONE) {
                throw std::runtime_error(sqlite3_errstr(err_code));
            }
        }
        execute("COMMIT;");
    } catch (const std::exception& e) {
        sqlite3_finalize(stmt);
        try { execute("ROLLBACK;"); } catch (...) {}
        throw;
    }

    sqlite3_finalize(stmt);
}

[[nodiscard]] auto SQLiteDatabase::query(
        const std::string& query
) -> QueryResult {
    QueryResult results;
    sqlite3_stmt* stmt = nullptr;
    
    if (int err_code = sqlite3_prepare_v2(
                pimpl_->db,
                query.c_str(),
                -1,
                &stmt,
                nullptr
        ); err_code != SQLITE_OK
    ) {
        throw std::runtime_error(sqlite3_errstr(err_code));
    }

    int cols = sqlite3_column_count(stmt);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Row row;

        for (int i = 0; i < cols; i++) {
            if (sqlite3_column_type(stmt, i) == SQLITE_NULL) {
                row.emplace_back("");
            } else {
                const unsigned char* str_value = sqlite3_column_text(stmt, i);
                row.emplace_back(reinterpret_cast<const char*>(str_value));
            }
        }

        results.push_back(row);
    }

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errstr(rc));
    }

    sqlite3_finalize(stmt);
    return results;
}

[[nodiscard]] auto SQLiteDatabase::query(
        const std::string& query,
        const Row& params
) -> QueryResult {
    QueryResult results;
    sqlite3_stmt* stmt = nullptr;

    if (int err_code = sqlite3_prepare_v2(
                pimpl_->db,
                query.c_str(),
                -1,
                &stmt,
                nullptr
        ); err_code != SQLITE_OK
    ) {
        throw std::runtime_error(sqlite3_errstr(err_code));
    }

    for (size_t i = 0; i < params.size(); ++i) {
        if (int err_code = sqlite3_bind_text(
                    stmt,
                    static_cast<int>(i + 1),
                    params[i].c_str(),
                    -1,
                    SQLITE_TRANSIENT
            ); err_code != SQLITE_OK
        ) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(sqlite3_errstr(err_code));
        }
    }

    int cols = sqlite3_column_count(stmt);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Row row;
        for (int i = 0; i < cols; i++) {
            if (sqlite3_column_type(stmt, i) == SQLITE_NULL) {
                row.emplace_back("");
            } else {
                const unsigned char* str_value = sqlite3_column_text(stmt, i);
                row.emplace_back(reinterpret_cast<const char*>(str_value));
            }
        }
        results.push_back(row);
    }

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(sqlite3_errstr(rc));
    }

    sqlite3_finalize(stmt);
    return results;
}

