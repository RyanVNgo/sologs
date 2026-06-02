
#include <filesystem>
#include <format>

#include <gtest/gtest.h>

#include "db_utils.h"

#include "db/auth_repository.h"


TEST(AuthRepo, init) {
    const char* db_filename = "auth_repo_init.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);
    EXPECT_TRUE(util_table_exists(db.get(), "auth_keys"));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, insert) {
    const char* db_filename = "auth_repo_insert.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    AuthorizationEntry entry{
        .uuid = "a1b2c3d4",
        .key_hash = "abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890",
        .name = "test-key",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2027-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_TRUE(repo.insert(entry));
    EXPECT_TRUE(util_validate_row_count(db.get(), "auth_keys", 1));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, insert_batch) {
    const char* db_filename = "auth_repo_batch.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    auto db = util_open_database(db_filename);

    std::vector<AuthorizationEntry> entries;
    const size_t entry_count = 3;
    for (size_t i = 0; i < entry_count; ++i) {
        entries.push_back(AuthorizationEntry{
            .uuid = "uuid-" + std::to_string(i),
            .key_hash = "hash-" + std::to_string(i),
            .name = "key-" + std::to_string(i),
            .permissions = {Permissions::LogRead},
            .created_at = "2026-01-01 00:00:00",
            .expires_at = "2027-01-01 00:00:00",
            .is_valid = true
        });
    }

    EXPECT_TRUE(repo.insert_batch(entries));
    EXPECT_TRUE(util_validate_row_count(db.get(), "auth_keys", entry_count));

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, get_by_key_hash_found) {
    const char* db_filename = "auth_repo_get_found.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    AuthorizationEntry entry{
        .uuid = "test-uuid",
        .key_hash = "testhash123",
        .name = "test-key-name",
        .permissions = {Permissions::LogRead, Permissions::Admin},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2027-01-01 00:00:00",
        .is_valid = true
    };

    ASSERT_TRUE(repo.insert(entry));

    auto result = repo.get_by_key_hash("testhash123");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->uuid, "test-uuid");
    EXPECT_EQ(result->key_hash, "testhash123");
    EXPECT_EQ(result->name, "test-key-name");
    EXPECT_EQ(result->is_valid, true);
    EXPECT_EQ(result->created_at, "2026-01-01 00:00:00");
    EXPECT_EQ(result->expires_at, "2027-01-01 00:00:00");

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, get_by_key_hash_not_found) {
    const char* db_filename = "auth_repo_get_notfound.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    auto result = repo.get_by_key_hash("nonexistent_hash");
    EXPECT_FALSE(result.has_value());

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, permissions_round_trip) {
    const char* db_filename = "auth_repo_perms.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    std::vector<Permissions> expected_perms = {
        Permissions::LogRead,
        Permissions::LogWrite,
        Permissions::Admin
    };

    AuthorizationEntry entry{
        .uuid = "perms-uuid",
        .key_hash = "permshash",
        .name = "perms-key",
        .permissions = expected_perms,
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2027-01-01 00:00:00",
        .is_valid = true
    };

    ASSERT_TRUE(repo.insert(entry));

    auto result = repo.get_by_key_hash("permshash");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->permissions.size(), expected_perms.size());
    for (size_t i = 0; i < expected_perms.size(); ++i) {
        EXPECT_EQ(result->permissions[i], expected_perms[i]);
    }

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, is_valid_false) {
    const char* db_filename = "auth_repo_invalid.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    AuthorizationEntry entry{
        .uuid = "invalid-uuid",
        .key_hash = "invalidhash",
        .name = "invalid-key",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2027-01-01 00:00:00",
        .is_valid = false
    };

    ASSERT_TRUE(repo.insert(entry));

    auto result = repo.get_by_key_hash("invalidhash");
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->is_valid);

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, empty_permissions) {
    const char* db_filename = "auth_repo_empty_perms.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    AuthorizationEntry entry{
        .uuid = "empty-perms-uuid",
        .key_hash = "emptypermshash",
        .name = "empty-perms-key",
        .permissions = {},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2027-01-01 00:00:00",
        .is_valid = true
    };

    ASSERT_TRUE(repo.insert(entry));

    auto result = repo.get_by_key_hash("emptypermshash");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->permissions.empty());

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

TEST(AuthRepo, unrecognized_permission_skipped) {
    const char* db_filename = "auth_repo_bad_perm.sqlite";
    std::unique_ptr<SQLiteDatabase> test_db;
    ASSERT_NO_THROW(test_db = std::make_unique<SQLiteDatabase>(db_filename));
    SqlAuthRepository repo(*test_db.get());

    AuthorizationEntry entry{
        .uuid = "bad-perm-uuid",
        .key_hash = "badpermhash",
        .name = "bad-perm-key",
        .permissions = {Permissions::LogRead, Permissions::Admin},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2027-01-01 00:00:00",
        .is_valid = true
    };

    ASSERT_TRUE(repo.insert(entry));

    auto db = util_open_database(db_filename);

    std::string update_query = std::format(R"(
        UPDATE auth_keys
        SET permissions = 'BogusPerm,LogRead'
        WHERE key_hash = 'badpermhash';
    )");

    ASSERT_TRUE(util_exec_query(db.get(), update_query.c_str()));

    auto result = repo.get_by_key_hash("badpermhash");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->permissions.size(), 1);
    EXPECT_EQ(result->permissions[0], Permissions::LogRead);

    EXPECT_TRUE(std::filesystem::exists(db_filename));
    std::filesystem::remove(db_filename);
}

