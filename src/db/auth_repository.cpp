
#include "auth_repository.h"

#include <sstream>

#include "utils.h"


SqlAuthRepository::SqlAuthRepository(
        SQLiteDatabase& db
) : database_(db)
{
    const char* sql =
        "CREATE TABLE IF NOT EXISTS auth_keys ("
        "uuid TEXT PRIMARY KEY,"
        "key_hash TEXT NOT NULL UNIQUE,"
        "name TEXT NOT NULL,"
        "permissions TEXT NOT NULL,"
        "created_at TEXT NOT NULL,"
        "expires_at TEXT NOT NULL,"
        "is_valid INTEGER NOT NULL DEFAULT 1"
        ");";

    database_.execute(sql);
}

auto SqlAuthRepository::insert(const AuthorizationEntry& entry) -> void {
    const char* sql =
        "INSERT INTO auth_keys (uuid, key_hash, name, permissions, "
        "created_at, expires_at, is_valid) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    Row row_data;
    row_data.push_back(entry.uuid);
    row_data.push_back(entry.key_hash);
    row_data.push_back(entry.name);
    row_data.push_back(sologs::utils::permissions_to_string(entry.permissions));
    row_data.push_back(entry.created_at);
    row_data.push_back(entry.expires_at);
    row_data.push_back(entry.is_valid ? "1" : "0");

    database_.execute_prepared(sql, row_data);
}

auto SqlAuthRepository::insert_batch(
        const std::vector<AuthorizationEntry>& entries
) -> void {
    std::string sql =
        "INSERT INTO auth_keys (uuid, key_hash, name, permissions, "
        "created_at, expires_at, is_valid) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    std::vector<Row> data;
    for (const auto& entry : entries) {
        Row row_data;
        row_data.push_back(entry.uuid);
        row_data.push_back(entry.key_hash);
        row_data.push_back(entry.name);
        row_data.push_back(sologs::utils::permissions_to_string(entry.permissions));
        row_data.push_back(entry.created_at);
        row_data.push_back(entry.expires_at);
        row_data.push_back(entry.is_valid ? "1" : "0");
        data.push_back(row_data);
    }

    database_.execute_prepared_batched(sql, data);
}

auto SqlAuthRepository::get_auth_entries(
        const UserFilterParams& params
) -> std::vector<AuthorizationEntry> {
    std::ostringstream sql;
    sql << "SELECT uuid, key_hash, name, permissions, "
           "created_at, expires_at, is_valid "
           "FROM auth_keys";

    Row bound_params;
    bool has_where = false;

    auto add = [&](const std::string& clause, const std::string& value) {
        sql << (has_where ? " AND" : " WHERE") << clause;
        bound_params.push_back(value);
        has_where = true;
    };

    if (params.uuid.has_value()) {
        add(" uuid = ?", params.uuid.value());
    }

    if (params.name.has_value()) {
        add(" name = ?", params.name.value());
    }

    if (params.permissions.has_value()) {
        for (const auto& perm : params.permissions.value()) {
            add(" permissions LIKE ?",
                "%" + sologs::utils::permission_label(perm) + "%"
            );
        }
    }

    if (params.created_after.has_value()) {
        add(" created_at >= ?", params.created_after.value());
    }

    if (params.created_before.has_value()) {
        add(" created_at <= ?", params.created_before.value());
    }

    if (params.expires_after.has_value()) {
        add(" expires_at >= ?", params.expires_after.value());
    }

    if (params.expires_before.has_value()) {
        add(" expires_at <= ?", params.expires_before.value());
    }

    if (params.is_valid.has_value()) {
        add(" is_valid = ?", params.is_valid.value() ? "1" : "0");
    }

    sql << " LIMIT " << params.limit << ";";

    QueryResult results = database_.query(sql.str(), bound_params);

    std::vector<AuthorizationEntry> entries;
    entries.reserve(results.size());
    for (const auto& row : results) {
        entries.push_back({
            .uuid = row.at(0),
            .key_hash = row.at(1),
            .name = row.at(2),
            .permissions = sologs::utils::parse_permissions(row.at(3)),
            .created_at = row.at(4),
            .expires_at = row.at(5),
            .is_valid = (row.at(6) == "1")
        });
    }

    return entries;
}

auto SqlAuthRepository::get_by_key_hash(
    const std::string& hash
) const -> std::optional<AuthorizationEntry> {
    const char* sql =
        "SELECT uuid, key_hash, name, permissions, "
        "created_at, expires_at, is_valid "
        "FROM auth_keys WHERE key_hash = ?;";

    Row params;
    params.push_back(hash);
    QueryResult results = database_.query(sql, params);

    if (results.empty()) {
        return {};
    }

    const auto& row = results[0];
    AuthorizationEntry entry;
    entry.uuid = row.at(0);
    entry.key_hash = row.at(1);
    entry.name = row.at(2);
    entry.permissions = sologs::utils::parse_permissions(row.at(3));
    entry.created_at = row.at(4);
    entry.expires_at = row.at(5);
    entry.is_valid = (row.at(6) == "1");

    return entry;
}

auto SqlAuthRepository::has_any_admin() const -> bool {
    const char* sql = 
        "SELECT COUNT(*) FROM auth_keys "
        "WHERE permissions LIKE '%Admin%';";

    QueryResult results = database_.query(sql, {});

    if (results.empty()) {
        return false;
    }

    int count = std::stoi(results[0].at(0));
    return count > 0;
}

