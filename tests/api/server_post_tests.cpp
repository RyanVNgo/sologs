
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

TEST(Server, post_valid) {
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

    EXPECT_CALL(mock_service, create_log).Times(testing::Exactly(1));
    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Post(
            "/logs",
            headers,
            R"(
                {
                    "message": "test log",
                    "level": "INFO",
                    "source": "sologs-benchmark"
                }
            )",
            "application/json"
    );

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 202);

    server.stop();
    t.join();
}

TEST(Server, post_invalid) {
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

    EXPECT_CALL(mock_service, create_log).Times(testing::Exactly(0));
    httplib::Headers headers = {{"Authorization", "Bearer test-key"}};
    auto res = client.Post(
            "/logs",
            headers,
            R"(
                {
                    "message": "test log",
                    "level": "INFO",
            )",
            "application/json"
    );

    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 400);

    server.stop();
    t.join();
}


