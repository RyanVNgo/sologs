
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <nlohmann/json.hpp>

#include <server/server.h>
#include <service/user_service.h>
#include <database/auth_repository.h>
#include <database/database.h>
#include <crypto/crypto.h>

using json = nlohmann::json;


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(void, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (LogFilterParams params), (const override));
};


static auto make_get_request(
        SOLogSServer& server,
        const std::string& key,
        const std::vector<std::pair<std::string, std::string>>& params = {}
) -> drogon::HttpResponsePtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setMethod(drogon::Get);
    if (!key.empty())
        req->addHeader("Authorization", "Bearer " + key);
    for (const auto& [k, v] : params)
        req->setParameter(k, v);

    drogon::HttpResponsePtr resp;
    server.get_auth_handler(
        req,
        [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );
    return resp;
}

static auto make_post_request(
        SOLogSServer& server,
        const std::string& key,
        const json& body
) -> drogon::HttpResponsePtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setMethod(drogon::Post);
    if (!key.empty())
        req->addHeader("Authorization", "Bearer " + key);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody(body.dump());

    drogon::HttpResponsePtr resp;
    server.post_auth_handler(
        req,
        [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );
    return resp;
}


TEST(AuthE2E, get_users_no_filters) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto r1 = auth_service.create_user(
        "Alice", {Permissions::Admin}, "9999-12-31 23:59:59"
    );
    (void)auth_service.create_user(
        "Bob", {Permissions::LogRead}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_request(server, r1.raw_key);
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 2);
    EXPECT_EQ(body[0]["name"], "Alice");
    EXPECT_EQ(body[1]["name"], "Bob");
}

TEST(AuthE2E, get_users_filter_name) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto r1 = auth_service.create_user(
        "Alice", {Permissions::Admin}, "9999-12-31 23:59:59"
    );
    (void)auth_service.create_user(
        "Bob", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_request(server, r1.raw_key, {{"name", "Alice"}});
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 1);
    EXPECT_EQ(body[0]["name"], "Alice");
}

TEST(AuthE2E, get_users_filter_permissions) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto r1 = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );
    (void)auth_service.create_user(
        "Reader", {Permissions::LogRead}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_request(server, r1.raw_key, {{"permissions", "Admin"}});
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 1);
    EXPECT_EQ(body[0]["name"], "Admin");
}

TEST(AuthE2E, get_users_filter_time_range) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_request(
        server, admin.raw_key,
        {{"created_after", "2025-01-01T00:00:00Z"}}
    );
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 1);
    EXPECT_EQ(body[0]["name"], "Admin");
}

TEST(AuthE2E, get_users_no_match) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_request(server, admin.raw_key, {{"name", "Nobody"}});
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    EXPECT_TRUE(body.empty());
}

TEST(AuthE2E, get_users_unauthorized) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto resp = make_get_request(server, "");
    EXPECT_EQ(resp->statusCode(), drogon::k401Unauthorized);
}

TEST(AuthE2E, get_users_forbidden) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto r1 = auth_service.create_user(
        "Reader", {Permissions::LogRead}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_request(server, r1.raw_key);
    EXPECT_EQ(resp->statusCode(), drogon::k403Forbidden);
}

TEST(AuthE2E, get_users_expired_key) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    std::string raw_key = "expired-test-key";
    std::string key_hash = sologs::crypto::sha256_hex(raw_key);

    AuthorizationEntry entry{
        .uuid = "expired-uuid",
        .key_hash = key_hash,
        .name = "Expired",
        .permissions = {Permissions::Admin},
        .created_at = "2020-01-01 00:00:00",
        .expires_at = "2020-06-01 00:00:00",
        .is_valid = true
    };
    auth_repo.insert(entry);

    auto resp = make_get_request(server, raw_key);
    EXPECT_EQ(resp->statusCode(), drogon::k401Unauthorized);
}

TEST(AuthE2E, get_users_invalid_key) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    std::string raw_key = "invalid-test-key";
    std::string key_hash = sologs::crypto::sha256_hex(raw_key);

    AuthorizationEntry entry{
        .uuid = "invalid-uuid",
        .key_hash = key_hash,
        .name = "Invalid",
        .permissions = {Permissions::Admin},
        .created_at = "2026-01-01 00:00:00",
        .expires_at = "9999-12-31 23:59:59",
        .is_valid = false
    };
    auth_repo.insert(entry);

    auto resp = make_get_request(server, raw_key);
    EXPECT_EQ(resp->statusCode(), drogon::k401Unauthorized);
}

TEST(AuthE2E, get_users_bad_datetime) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_request(
        server, admin.raw_key, {{"created_after", "not-a-date"}}
    );
    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

TEST(AuthE2E, post_auth_success) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"name", "new-user"},
        {"permissions", {"LogRead"}}
    };

    auto resp = make_post_request(server, admin.raw_key, body);
    ASSERT_EQ(resp->statusCode(), drogon::k201Created);

    json data = json::parse(resp->body());
    EXPECT_TRUE(data.contains("key"));
    EXPECT_TRUE(data.contains("uuid"));
    EXPECT_EQ(data["name"], "new-user");
    EXPECT_TRUE(data.contains("created_at"));
    EXPECT_EQ(data["expires_at"], "9999-12-31 23:59:59");
    EXPECT_EQ(data["is_valid"], true);
    ASSERT_EQ(data["permissions"].size(), 1);
    EXPECT_EQ(data["permissions"][0], "LogRead");
    EXPECT_GT(data["key"].get<std::string>().size(), 0);
}

TEST(AuthE2E, post_auth_custom_expiry) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"name", "temporary"},
        {"permissions", {"LogRead"}},
        {"expires_at", "2027-06-01 00:00:00"}
    };

    auto resp = make_post_request(server, admin.raw_key, body);
    ASSERT_EQ(resp->statusCode(), drogon::k201Created);

    json data = json::parse(resp->body());
    EXPECT_EQ(data["expires_at"], "2027-06-01 00:00:00");
}

TEST(AuthE2E, post_auth_single_permission) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"name", "single-perm"},
        {"permissions", {"LogWrite"}}
    };

    auto resp = make_post_request(server, admin.raw_key, body);
    ASSERT_EQ(resp->statusCode(), drogon::k201Created);

    json data = json::parse(resp->body());
    ASSERT_EQ(data["permissions"].size(), 1);
    EXPECT_EQ(data["permissions"][0], "LogWrite");
}

TEST(AuthE2E, post_auth_multiple_permissions) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"name", "multi-perm"},
        {"permissions", {"LogRead", "LogWrite", "Admin"}}
    };

    auto resp = make_post_request(server, admin.raw_key, body);
    ASSERT_EQ(resp->statusCode(), drogon::k201Created);

    json data = json::parse(resp->body());
    ASSERT_EQ(data["permissions"].size(), 3);
    EXPECT_EQ(data["permissions"][0], "LogRead");
    EXPECT_EQ(data["permissions"][1], "LogWrite");
    EXPECT_EQ(data["permissions"][2], "Admin");
}

TEST(AuthE2E, post_auth_then_get) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json post_body = {
        {"name", "NewUser"},
        {"permissions", {"LogRead"}}
    };

    auto post_resp = make_post_request(server, admin.raw_key, post_body);
    ASSERT_EQ(post_resp->statusCode(), drogon::k201Created);
    json post_data = json::parse(post_resp->body());

    auto get_resp = make_get_request(
        server, admin.raw_key, {{"name", "NewUser"}}
    );
    ASSERT_EQ(get_resp->statusCode(), drogon::k200OK);
    json get_data = json::parse(get_resp->body());
    ASSERT_EQ(get_data.size(), 1);
    EXPECT_EQ(get_data[0]["name"], "NewUser");
    EXPECT_EQ(get_data[0]["uuid"], post_data["uuid"]);
    ASSERT_EQ(get_data[0]["permissions"], "LogRead");
}

TEST(AuthE2E, post_auth_unauthorized) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    json body = {
        {"name", "ignored"},
        {"permissions", {"LogRead"}}
    };

    auto resp = make_post_request(server, "", body);
    EXPECT_EQ(resp->statusCode(), drogon::k401Unauthorized);
}

TEST(AuthE2E, post_auth_forbidden) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto reader = auth_service.create_user(
        "Reader", {Permissions::LogRead}, "9999-12-31 23:59:59"
    );

    json body = {
        {"name", "ignored"},
        {"permissions", {"LogRead"}}
    };

    auto resp = make_post_request(server, reader.raw_key, body);
    EXPECT_EQ(resp->statusCode(), drogon::k403Forbidden);
}

TEST(AuthE2E, post_auth_invalid_json) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer " + admin.raw_key);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody("{invalid json");

    drogon::HttpResponsePtr resp;
    server.post_auth_handler(
        req,
        [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

TEST(AuthE2E, post_auth_missing_name) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"permissions", {"LogRead"}}
    };

    auto resp = make_post_request(server, admin.raw_key, body);
    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

TEST(AuthE2E, post_auth_missing_permissions) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"name", "no-perms"}
    };

    auto resp = make_post_request(server, admin.raw_key, body);
    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

