
#include <optional>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "api/server.h"


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(void, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (LogFilterParams params), (const override));
};

class AuthServiceMock : public IAuthService {
    public:
        MOCK_METHOD(
            CreateUserResult,
            create_user,
            (
                const std::string& name,
                const std::vector<Permissions>& permissions,
                const std::string& expires_at
            ),
            (override)
        );

        MOCK_METHOD(
                json,
                get_users,
                (const UserFilterParams& params),
                (override)
        );

        MOCK_METHOD(
                std::optional<User>,
                authenticate,
                (const std::string& key),
                (override)
        );

        MOCK_METHOD(
            bool,
            subject_has_permissions,
            (
                const User& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
            ),
            (const override)
        );

};

TEST(Server, get_users_defaults) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([](const UserFilterParams& p) {
            return (!p.uuid.has_value() && 
                    !p.name.has_value() &&
                    !p.permissions.has_value() &&
                    !p.created_after.has_value() &&
                    !p.created_before.has_value() &&
                    !p.expires_after.has_value() &&
                    !p.expires_before.has_value() &&
                    !p.is_valid.has_value() &&
                    p.limit == 100);
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_uuid) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    std::string check_uuid = "check_uuid";

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.uuid.has_value() && p.uuid == check_uuid;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("uuid", check_uuid);
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_name) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    std::string check_name = "check_name";

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.name.has_value() && p.name == check_name;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("name", check_name);
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_permissions) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    std::vector<Permissions> check_perms = {
        Permissions::Admin,
        Permissions::LogDelete,
    };

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            if (!p.permissions.has_value()) {
                return false;
            }
            auto& p_val = p.permissions.value();
            for (const auto& perm : check_perms) {
                auto it = std::find(p_val.begin(), p_val.end(), perm);
                if (it == p_val.end()) {
                    return false;
                }
            }
            return true;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("permissions", "Admin,LogDelete");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_created_after) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    std::string check_time = "2027-01-01T00:00:00Z";

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.created_after.has_value() && p.created_after == check_time;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("created_after", check_time);
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_created_before) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    std::string check_time = "2027-01-01T00:00:00Z";

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.created_before.has_value() && p.created_before == check_time;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("created_before", check_time);
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_expires_after) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    std::string check_time = "2027-01-01T00:00:00Z";

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.expires_after.has_value() && p.expires_after == check_time;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("expires_after", check_time);
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_expires_before) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    std::string check_time = "2027-01-01T00:00:00Z";

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.expires_before.has_value() && p.expires_before == check_time;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("expires_before", check_time);
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_is_valid) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    bool check_is_valid = true;

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.is_valid.has_value() && p.is_valid == check_is_valid;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("is_valid", "true");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_users_limit) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<User>(
            User{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    int check_limit = 711;

    EXPECT_CALL(mock_auth_service, get_users(
        testing::Truly([&](const UserFilterParams& p) {
            return p.limit == check_limit;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setParameter("limit", "711");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

