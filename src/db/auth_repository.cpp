
#include "auth_repository.h"

#include <sstream>


static std::string permission_label(Permissions perm) {
    switch (perm) {
        case Permissions::LogRead:    return "LogRead";
        case Permissions::LogWrite:   return "LogWrite";
        case Permissions::LogDelete:  return "LogDelete";
        case Permissions::AuthRead:   return "AuthRead";
        case Permissions::AuthWrite:  return "AuthWrite";
        case Permissions::AuthDelete: return "AuthDelete";
        case Permissions::Admin:      return "Admin";
    }
    return "";
}

static std::optional<Permissions> permission_from_label(const std::string& label) {
    if (label == "LogRead")    return Permissions::LogRead;
    if (label == "LogWrite")   return Permissions::LogWrite;
    if (label == "LogDelete")  return Permissions::LogDelete;
    if (label == "AuthRead")   return Permissions::AuthRead;
    if (label == "AuthWrite")  return Permissions::AuthWrite;
    if (label == "AuthDelete") return Permissions::AuthDelete;
    if (label == "Admin")      return Permissions::Admin;
    return {};
}

static std::string permissions_to_string(const std::vector<Permissions>& perms) {
    std::ostringstream oss;
    for (size_t i = 0; i < perms.size(); ++i) {
        if (i > 0) oss << ",";
        oss << permission_label(perms[i]);
    }
    return oss.str();
}

static std::vector<Permissions> parse_permissions(const std::string& str) {
    std::vector<Permissions> result;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        if (token.empty()) continue;
        auto perm = permission_from_label(token);
        if (perm.has_value()) {
            result.push_back(perm.value());
        }
    }
    return result;
}

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

auto SqlAuthRepository::insert(const AuthorizationEntry& entry) -> bool {
    const char* sql =
        "INSERT INTO auth_keys (uuid, key_hash, name, permissions, "
        "created_at, expires_at, is_valid) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    Row row_data;
    row_data.push_back(entry.uuid);
    row_data.push_back(entry.key_hash);
    row_data.push_back(entry.name);
    row_data.push_back(permissions_to_string(entry.permissions));
    row_data.push_back(entry.created_at);
    row_data.push_back(entry.expires_at);
    row_data.push_back(entry.is_valid ? "1" : "0");

    return database_.execute_prepared(sql, row_data);
}

auto SqlAuthRepository::insert_batch(
        const std::vector<AuthorizationEntry>& entries
) -> bool {
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
        row_data.push_back(permissions_to_string(entry.permissions));
        row_data.push_back(entry.created_at);
        row_data.push_back(entry.expires_at);
        row_data.push_back(entry.is_valid ? "1" : "0");
        data.push_back(row_data);
    }

    return database_.execute_prepared_batched(sql, data);
}

auto SqlAuthRepository::get_by_key_hash(
    const std::string& hash
) -> std::optional<AuthorizationEntry> {
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
    entry.permissions = parse_permissions(row.at(3));
    entry.created_at = row.at(4);
    entry.expires_at = row.at(5);
    entry.is_valid = (row.at(6) == "1");

    return entry;
}

auto SqlAuthRepository::has_any_admin() -> bool {
    const char* sql = 
        "SELECT COUNT(*) FROM auth_keys"
        "WHERE permissions LIKE '%Admin%';";

    QueryResult results = database_.query(sql, {});

    if (results.empty()) {
        return false;
    }

    int count = std::stoi(results[0].at(0));
    return count > 0;
}

