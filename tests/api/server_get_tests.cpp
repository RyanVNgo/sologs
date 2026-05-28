
#include <optional>
#include <thread>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "api/server.h"


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(bool, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (FilterParams params), (override));
};

class AuthorizerMock : public IAuthorizer {
    public:
        MOCK_METHOD(
            bool,
            has_permissions,
            (
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
            ),
            (const override)
        );
};

class AuthenticatorMock : public IAuthenticator {
    public:
        MOCK_METHOD(
                std::optional<Subject>,
                authenticate,
                (const std::string& key),
                (const override)
        );
};

TEST(Server, get_health) {
    LogServiceMock mock_service;
    AuthorizerMock mock_authorizer;
    AuthenticatorMock mock_authenticator;
    SOLogSServer server(mock_service, mock_authorizer, mock_authenticator);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    auto res = client.Get("/health");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}

TEST(Server, get_logs) {
    LogServiceMock mock_service;
    AuthorizerMock mock_authorizer;
    AuthenticatorMock mock_authenticator;
    SOLogSServer server(mock_service, mock_authorizer, mock_authenticator);

    const int port = 8080;
    std::thread t([&]() {
        server.start(port);
    });

    httplib::Client client("localhost", port);

    EXPECT_CALL(mock_authenticator, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_authorizer, has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_service, get_logs).Times(testing::Exactly(1));
    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Get("/logs", headers);

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}

TEST(Server, get_logs_with_level_param) {
    LogServiceMock mock_service;
    AuthorizerMock mock_authorizer;
    AuthenticatorMock mock_authenticator;
    SOLogSServer server(mock_service, mock_authorizer, mock_authenticator);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    EXPECT_CALL(mock_authenticator, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_authorizer, has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const FilterParams& p) {
            return p.level.has_value() && p.level.value() == "ERROR";
        })
    )).Times(testing::Exactly(1));

    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Get("/logs?level=ERROR", headers);

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}

TEST(Server, get_logs_with_source_param) {
    LogServiceMock mock_service;
    AuthorizerMock mock_authorizer;
    AuthenticatorMock mock_authenticator;
    SOLogSServer server(mock_service, mock_authorizer, mock_authenticator);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    EXPECT_CALL(mock_authenticator, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_authorizer, has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const FilterParams& p) {
            return p.source.has_value() && p.source.value() == "api";
        })
    )).Times(testing::Exactly(1));

    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Get("/logs?source=api", headers);

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}

TEST(Server, get_logs_with_limit_param) {
    LogServiceMock mock_service;
    AuthorizerMock mock_authorizer;
    AuthenticatorMock mock_authenticator;
    SOLogSServer server(mock_service, mock_authorizer, mock_authenticator);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    EXPECT_CALL(mock_authenticator, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_authorizer, has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const FilterParams& p) {
            return p.limit.has_value() && p.limit.value() == 10;
        })
    )).Times(testing::Exactly(1));

    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Get("/logs?limit=10", headers);

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}

TEST(Server, get_logs_with_multiple_params) {
    LogServiceMock mock_service;
    AuthorizerMock mock_authorizer;
    AuthenticatorMock mock_authenticator;
    SOLogSServer server(mock_service, mock_authorizer, mock_authenticator);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    EXPECT_CALL(mock_authenticator, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_authorizer, has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const FilterParams& p) {
            return p.level.value() == "WARN" && p.source.value() == "cli" && p.limit.value() == 50;
        })
    )).Times(testing::Exactly(1));

    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Get("/logs?level=WARN&source=cli&limit=50", headers);

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}

TEST(Server, get_logs_with_invalid_limit) {
    LogServiceMock mock_service;
    AuthorizerMock mock_authorizer;
    AuthenticatorMock mock_authenticator;
    SOLogSServer server(mock_service, mock_authorizer, mock_authenticator);

    std::thread t([&]() {
        server.start(8080);
    });

    httplib::Client client("localhost", 8080);

    EXPECT_CALL(mock_authenticator, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_authorizer, has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_service, get_logs(
        testing::Truly([](const FilterParams& p) {
            return !p.limit.has_value();
        })
    )).Times(testing::Exactly(1));

    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Get("/logs?limit=abc", headers);

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);

    server.stop();
    t.join();
}



