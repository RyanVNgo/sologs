
#include <optional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "service/auth_service.h"


class AuthRepositoryMock : public IAuthRepository {
    public:
        MOCK_METHOD(void, insert, (const AuthorizationEntry&), (override));
        MOCK_METHOD(void, insert_batch, (const std::vector<AuthorizationEntry>&), (override));
        MOCK_METHOD(std::optional<AuthorizationEntry>, get_by_key_hash, (const std::string&), (const override));
        MOCK_METHOD(bool, has_any_admin, (), (const override));
};


TEST(Authorizer, has_permissions_anyof_granted) {
    Authorizer authz;
    Subject subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_TRUE(authz.has_permissions(
        subject, {Permissions::LogRead}, PermissionMode::AnyOf
    ));
}

TEST(Authorizer, has_permissions_anyof_denied) {
    Authorizer authz;
    Subject subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_FALSE(authz.has_permissions(
        subject, {Permissions::LogWrite}, PermissionMode::AnyOf
    ));
}

TEST(Authorizer, has_permissions_anyof_partial_match) {
    Authorizer authz;
    Subject subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_TRUE(authz.has_permissions(
        subject,
        {Permissions::LogWrite, Permissions::Admin, Permissions::LogRead},
        PermissionMode::AnyOf
    ));
}

TEST(Authorizer, has_permissions_allof_granted) {
    Authorizer authz;
    Subject subject{"uuid", "test", {Permissions::LogRead, Permissions::LogWrite}};

    EXPECT_TRUE(authz.has_permissions(
        subject,
        {Permissions::LogRead, Permissions::LogWrite},
        PermissionMode::AllOf
    ));
}

TEST(Authorizer, has_permissions_allof_denied) {
    Authorizer authz;
    Subject subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_FALSE(authz.has_permissions(
        subject,
        {Permissions::LogRead, Permissions::LogWrite},
        PermissionMode::AllOf
    ));
}

TEST(Authorizer, has_permissions_allof_extra_permissions) {
    Authorizer authz;
    Subject subject{"uuid", "test", {Permissions::LogRead, Permissions::LogWrite, Permissions::Admin}};

    EXPECT_TRUE(authz.has_permissions(
        subject, {Permissions::LogRead}, PermissionMode::AllOf
    ));
}

TEST(Authorizer, has_permissions_empty_required) {
    Authorizer authz;
    Subject subject{"uuid", "test", {Permissions::LogRead}};

    EXPECT_FALSE(authz.has_permissions(
        subject, {}, PermissionMode::AnyOf
    ));
}


TEST(Authenticator, authenticate_valid_key) {
    AuthRepositoryMock mock_repo;
    Authenticator authn(mock_repo);

    AuthorizationEntry entry{
        .uuid = "test-uuid",
        .key_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        .name = "test-key",
        .permissions = {Permissions::LogRead, Permissions::LogWrite},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authn.authenticate("some-key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->uuid, "test-uuid");
    EXPECT_EQ(result->name, "test-key");
    ASSERT_EQ(result->permissions.size(), 2);
    EXPECT_EQ(result->permissions[0], Permissions::LogRead);
    EXPECT_EQ(result->permissions[1], Permissions::LogWrite);
}

TEST(Authenticator, authenticate_unknown_key) {
    AuthRepositoryMock mock_repo;
    Authenticator authn(mock_repo);

    EXPECT_CALL(mock_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::nullopt));

    auto result = authn.authenticate("unknown-key");
    EXPECT_FALSE(result.has_value());
}

TEST(Authenticator, authenticate_expired_key) {
    AuthRepositoryMock mock_repo;
    Authenticator authn(mock_repo);

    AuthorizationEntry entry{
        .uuid = "expired-uuid",
        .key_hash = "hash",
        .name = "expired-key",
        .permissions = {Permissions::LogRead},
        .created_at = "2020-01-01 00:00:00",
        .expires_at = "2020-06-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authn.authenticate("expired-key");
    EXPECT_FALSE(result.has_value());
}

TEST(Authenticator, authenticate_invalid_key) {
    AuthRepositoryMock mock_repo;
    Authenticator authn(mock_repo);

    AuthorizationEntry entry{
        .uuid = "invalid-uuid",
        .key_hash = "hash",
        .name = "invalid-key",
        .permissions = {Permissions::LogRead},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = false
    };

    EXPECT_CALL(mock_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authn.authenticate("invalid-key");
    EXPECT_FALSE(result.has_value());
}

TEST(Authenticator, authenticate_multiple_permissions) {
    AuthRepositoryMock mock_repo;
    Authenticator authn(mock_repo);

    AuthorizationEntry entry{
        .uuid = "multi-perm-uuid",
        .key_hash = "hash",
        .name = "multi-perm-key",
        .permissions = {Permissions::LogRead, Permissions::LogWrite, Permissions::Admin},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "2030-01-01 00:00:00",
        .is_valid = true
    };

    EXPECT_CALL(mock_repo, get_by_key_hash(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<AuthorizationEntry>(entry)));

    auto result = authn.authenticate("multi-perm-key");
    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->permissions.size(), 3);
    EXPECT_EQ(result->permissions[0], Permissions::LogRead);
    EXPECT_EQ(result->permissions[1], Permissions::LogWrite);
    EXPECT_EQ(result->permissions[2], Permissions::Admin);
}

