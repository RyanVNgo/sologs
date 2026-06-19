
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "api/server.h"


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(void, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (FilterParams params), (const override));
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
                std::optional<Subject>,
                authenticate,
                (const std::string& key),
                (override)
        );

        MOCK_METHOD(
            bool,
            subject_has_permissions,
            (
                const Subject& subject,
                const std::vector<Permissions>& valid_permissions,
                PermissionMode mode
            ),
            (const override)
        );

};

TEST(Server, post_valid) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));
    EXPECT_CALL(mock_service, create_log).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setBody(
            R"(
                {
                    "message": "test log",
                    "level": "INFO",
                    "source": "sologs-benchmark"
                }
            )"
    );
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.post_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k202Accepted);
}

TEST(Server, post_invalid) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));
    EXPECT_CALL(mock_service, create_log).Times(testing::Exactly(0));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setBody(
            R"(
                {
                    "message": "test log",
                    "level": "INFO",
            )"
    );
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.post_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

TEST(Server, post_auth_valid_json) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    const std::vector<Permissions> exp_perms = {
        Permissions::LogRead,
        Permissions::LogWrite
    };
    EXPECT_CALL(mock_auth_service, create_user(
        "test_name", exp_perms, "2027-01-01 00:00:00"
    )).Times(testing::Exactly(1));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setBody(
            R"(
                {
                    "name": "test_name",
                    "permissions": ["LogRead", "LogWrite"],
                    "expires_at": "2027-01-01 00:00:00"
                }
            )"
    );
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.post_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k201Created);
}

TEST(Server, post_auth_invalid_json) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_auth_service, create_user(
        testing::_,testing::_,testing::_
    )).Times(testing::Exactly(0));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setBody(
            R"(
                {
                    "name": "test_name",
                    "permissions": ["LogRead", "LogWrite"],
            )"
    );
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.post_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

TEST(Server, post_auth_missing_field) {
    LogServiceMock mock_service;
    AuthServiceMock mock_auth_service;
    SOLogSServer server(
            mock_service,
            mock_auth_service
    );

    EXPECT_CALL(mock_auth_service, authenticate(testing::_))
        .Times(1)
        .WillOnce(testing::Return(std::optional<Subject>(
            Subject{"uuid", "name", {}}
        )));
    EXPECT_CALL(mock_auth_service, subject_has_permissions(
        testing::_, testing::_, testing::_
    )).Times(1).WillOnce(testing::Return(true));

    EXPECT_CALL(mock_auth_service, create_user(
        testing::_,testing::_,testing::_
    )).Times(testing::Exactly(0));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setBody(
            R"(
                {
                    "permissions": ["LogRead", "LogWrite"]
                    "expires_at": "2027-01-01 00:00:00"
                }
            )"
    );
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer test-key");

    drogon::HttpResponsePtr resp;
    server.post_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

