
#include <optional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "api/server.h"


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(void, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (LogFilterParams params), (const override));
};

class UserServiceMock : public IUserService {
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

TEST(Server, get_health) {
    LogServiceMock mock_service;
    UserServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/path");

    drogon::HttpResponsePtr resp;
    server.get_health(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_logs) {
    LogServiceMock mock_service;
    UserServiceMock mock_auth_service;
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

    EXPECT_CALL(mock_service, get_logs).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_logs_with_level_param) {
    LogServiceMock mock_service;
    UserServiceMock mock_auth_service;
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

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const LogFilterParams& p) {
            return p.level.has_value() && p.level.value() == "ERROR";
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setParameter("level", "ERROR");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_logs_with_source_param) {
    LogServiceMock mock_service;
    UserServiceMock mock_auth_service;
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

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const LogFilterParams& p) {
            return p.source.has_value() && p.source.value() == "api";
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setParameter("source", "api");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_logs_with_limit_param) {
    LogServiceMock mock_service;
    UserServiceMock mock_auth_service;
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

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const LogFilterParams& p) {
            return p.limit.has_value() && p.limit.value() == 10;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setParameter("limit", "10");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_logs_with_multiple_params) {
    LogServiceMock mock_service;
    UserServiceMock mock_auth_service;
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

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const LogFilterParams& p) {
            return p.level.value() == "WARN" && p.source.value() == "cli" && p.limit.value() == 50;
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setParameter("level", "WARN");
    req->setParameter("source", "cli");
    req->setParameter("limit", "50");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

TEST(Server, get_logs_with_invalid_limit) {
    LogServiceMock mock_service;
    UserServiceMock mock_auth_service;
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

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const LogFilterParams& p) {
            return !p.limit.has_value();
        })
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setParameter("limit", "abc");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.get_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k200OK);
}

