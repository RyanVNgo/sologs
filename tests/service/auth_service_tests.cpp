
#include <optional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <service/user_service.h>


class AuthRepositoryMock : public IAuthRepository {
    public:
        MOCK_METHOD(void, insert, (const AuthorizationEntry&), (override));
        MOCK_METHOD(void, insert_batch, (const std::vector<AuthorizationEntry>&), (override));
        MOCK_METHOD(std::vector<AuthorizationEntry>, get_auth_entries, (const UserFilterParams& params), (override));
        MOCK_METHOD(std::optional<AuthorizationEntry>, get_by_key_hash, (const std::string&), (const override));
        MOCK_METHOD(bool, has_any_admin, (), (const override));
};


TEST(UserService, has_permissions_anyof_granted) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);
    User subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_TRUE(authz.subject_has_permissions(
        subject, {Permissions::LogRead}, PermissionMode::AnyOf
    ));
}

TEST(UserService, has_permissions_anyof_denied) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);
    User subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_FALSE(authz.subject_has_permissions(
        subject, {Permissions::LogWrite}, PermissionMode::AnyOf
    ));
}

TEST(UserService, has_permissions_anyof_partial_match) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);
    User subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_TRUE(authz.subject_has_permissions(
        subject,
        {Permissions::LogWrite, Permissions::Admin, Permissions::LogRead},
        PermissionMode::AnyOf
    ));
}

TEST(UserService, has_permissions_allof_granted) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);
    User subject{"uuid", "test", {Permissions::LogRead, Permissions::LogWrite}};

    EXPECT_TRUE(authz.subject_has_permissions(
        subject,
        {Permissions::LogRead, Permissions::LogWrite},
        PermissionMode::AllOf
    ));
}

TEST(UserService, has_permissions_allof_denied) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);
    User subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_FALSE(authz.subject_has_permissions(
        subject,
        {Permissions::LogRead, Permissions::LogWrite},
        PermissionMode::AllOf
    ));
}

TEST(UserService, has_permissions_allof_extra_permissions) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);
    User subject{"uuid", "test", {Permissions::LogRead, Permissions::LogWrite, Permissions::Admin}};

    EXPECT_TRUE(authz.subject_has_permissions(
        subject, {Permissions::LogRead}, PermissionMode::AllOf
    ));
}

TEST(UserService, has_permissions_empty_required) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);
    User subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_FALSE(authz.subject_has_permissions(
        subject, {}, PermissionMode::AnyOf
    ));
}


TEST(UserService, authenticate_valid_key) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "test-uuid",
        .key_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        .name = "test-key",
        .permissions = {Permissions::LogRead, Permissions::LogWrite},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_auth_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authz.authenticate("some-key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->uuid, "test-uuid");
    EXPECT_EQ(result->name, "test-key");
    ASSERT_EQ(result->permissions.size(), 2);
    EXPECT_EQ(result->permissions[0], Permissions::LogRead);
    EXPECT_EQ(result->permissions[1], Permissions::LogWrite);
}

TEST(UserService, authenticate_unknown_key) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);

    EXPECT_CALL(mock_auth_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::nullopt));

    auto result = authz.authenticate("unknown-key");
    EXPECT_FALSE(result.has_value());
}

TEST(UserService, authenticate_expired_key) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "expired-uuid",
        .key_hash = "hash",
        .name = "expired-key",
        .permissions = {Permissions::LogRead},
        .created_at = "2020-01-01 00:00:00",
        .expires_at = "2020-06-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_auth_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authz.authenticate("expired-key");
    EXPECT_FALSE(result.has_value());
}

TEST(UserService, authenticate_invalid_key) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "invalid-uuid",
        .key_hash = "hash",
        .name = "invalid-key",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = false
    };

    EXPECT_CALL(mock_auth_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authz.authenticate("invalid-key");
    EXPECT_FALSE(result.has_value());
}

TEST(UserService, authenticate_multiple_permissions) {
    AuthRepositoryMock mock_auth_repo;
	UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "multi-perm-uuid",
        .key_hash = "hash",
        .name = "multi-perm-key",
        .permissions = {Permissions::LogRead, Permissions::LogWrite, Permissions::Admin},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_auth_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authz.authenticate("multi-perm-key");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->permissions.size(), 3);
    EXPECT_EQ(result->permissions[0], Permissions::LogRead);
    EXPECT_EQ(result->permissions[1], Permissions::LogWrite);
    EXPECT_EQ(result->permissions[2], Permissions::Admin);
}

TEST(UserService, get_users_empty) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    EXPECT_CALL(mock_auth_repo, get_auth_entries(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::vector<AuthorizationEntry>{}));

    json result = authz.get_users({});
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(result.size(), 0);
}

TEST(UserService, get_users_single_entry) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "user-1-uuid",
        .key_hash = "hash1",
        .name = "User One",
        .permissions = {Permissions::LogRead, Permissions::LogWrite},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_auth_repo, get_auth_entries(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::vector<AuthorizationEntry>{entry}));

    json result = authz.get_users({});
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]["uuid"], "user-1-uuid");
    EXPECT_EQ(result[0]["name"], "User One");
    EXPECT_EQ(result[0]["permissions"], "LogRead,LogWrite");
    EXPECT_EQ(result[0]["created_at"], "2026-01-01 00:00:00");
    EXPECT_EQ(result[0]["expires_at"], "2030-01-01 00:00:00");
    EXPECT_EQ(result[0]["is_valid"], "true");
}

TEST(UserService, get_users_multiple_entries) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    AuthorizationEntry entry1{
        .uuid = "uuid-1",
        .key_hash = "hash1",
        .name = "Alpha",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };
    AuthorizationEntry entry2{
        .uuid = "uuid-2",
        .key_hash = "hash2",
        .name = "Beta",
        .permissions = {Permissions::Admin},
        .created_at = "2026-06-01 00:00:00",
        .expires_at = "2030-06-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_auth_repo, get_auth_entries(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::vector<AuthorizationEntry>{entry1, entry2}));

    json result = authz.get_users({});
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0]["uuid"], "uuid-1");
    EXPECT_EQ(result[1]["uuid"], "uuid-2");
    EXPECT_EQ(result[0]["name"], "Alpha");
    EXPECT_EQ(result[1]["name"], "Beta");
    EXPECT_EQ(result[0]["permissions"], "LogRead");
    EXPECT_EQ(result[1]["permissions"], "Admin");
}

TEST(UserService, get_users_all_permission_types) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "all-perms-uuid",
        .key_hash = "hash",
        .name = "All Perms",
        .permissions = {
            Permissions::LogRead, Permissions::LogWrite, Permissions::LogDelete,
            Permissions::AuthRead, Permissions::AuthWrite, Permissions::AuthDelete,
            Permissions::Admin
        },
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_auth_repo, get_auth_entries(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::vector<AuthorizationEntry>{entry}));

    json result = authz.get_users({});
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]["permissions"], "LogRead,LogWrite,LogDelete,AuthRead,AuthWrite,AuthDelete,Admin");
}

TEST(UserService, get_users_invalid_entry) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "invalid-uuid",
        .key_hash = "hash",
        .name = "Invalid User",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2020-01-01 00:00:00",
        .is_valid = false
    };

    EXPECT_CALL(mock_auth_repo, get_auth_entries(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::vector<AuthorizationEntry>{entry}));

    json result = authz.get_users({});
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]["is_valid"], "false");
}

TEST(UserService, get_users_passthrough_params) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    UserFilterParams params{
        .uuid = std::nullopt,
        .name = "Specific User",
        .permissions = std::nullopt,
        .created_after = std::nullopt,
        .created_before = std::nullopt,
        .expires_after = std::nullopt,
        .expires_before = std::nullopt,
        .is_valid = std::nullopt,
        .limit = 10
    };

    AuthorizationEntry entry{
        .uuid = "filtered-uuid",
        .key_hash = "hash",
        .name = "Specific User",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_auth_repo, get_auth_entries(
        testing::Truly([](const UserFilterParams& p) {
            return p.name.has_value() && p.name.value() == "Specific User" && p.limit == 10;
        })
    )).Times(1).WillOnce(testing::Return(std::vector<AuthorizationEntry>{entry}));

    json result = authz.get_users(params);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]["name"], "Specific User");
}

TEST(UserService, get_users_invalid_created_after) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    UserFilterParams params;
    params.created_after = "not-a-date";
    params.limit = 100;

    EXPECT_CALL(mock_auth_repo, get_auth_entries(testing::_)).Times(0);
    EXPECT_THROW((void)authz.get_users(params), std::invalid_argument);
}

TEST(UserService, get_users_invalid_expires_before) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    UserFilterParams params;
    params.expires_before = "01/15/2026";
    params.limit = 100;

    EXPECT_CALL(mock_auth_repo, get_auth_entries(testing::_)).Times(0);
    EXPECT_THROW((void)authz.get_users(params), std::invalid_argument);
}

TEST(UserService, get_users_iso8601_normalized) {
    AuthRepositoryMock mock_auth_repo;
    UserService authz(mock_auth_repo);

    AuthorizationEntry entry{
        .uuid = "uuid",
        .key_hash = "hash",
        .name = "test",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    UserFilterParams params;
    params.created_after = "2026-06-01T12:30:00Z";
    params.limit = 100;

    EXPECT_CALL(mock_auth_repo, get_auth_entries(
        testing::Truly([](const UserFilterParams& p) {
            return p.created_after.has_value()
                && p.created_after.value() == "2026-06-01 12:30:00";
        })
    )).Times(1).WillOnce(testing::Return(std::vector<AuthorizationEntry>{entry}));

    json result = authz.get_users(params);
    ASSERT_EQ(result.size(), 1);
}

