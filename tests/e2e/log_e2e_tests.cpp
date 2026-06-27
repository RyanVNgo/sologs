
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <server/server.h>
#include <service/user_service.h>
#include <service/log_service.h>
#include <database/auth_repository.h>
#include <database/log_repository.h>
#include <database/database.h>

using json = nlohmann::json;


static auto make_post_log_request(
        SOLogSServer& server,
        const std::string& key,
        const json& body
) -> drogon::HttpResponsePtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer " + key);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody(body.dump());

    drogon::HttpResponsePtr resp;
    server.post_logs_handler(
        req,
        [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );
    return resp;
}

static auto make_get_log_request(
        SOLogSServer& server,
        const std::string& key,
        const std::vector<std::pair<std::string, std::string>>& params = {}
) -> drogon::HttpResponsePtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer " + key);
    for (const auto& [k, v] : params)
        req->setParameter(k, v);

    drogon::HttpResponsePtr resp;
    server.get_logs_handler(
        req,
        [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );
    return resp;
}


TEST(LogE2E, post_logs_success) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"message", "test log"},
        {"level", "INFO"},
        {"source", "e2e"}
    };

    auto resp = make_post_log_request(server, admin.raw_key, body);
    EXPECT_EQ(resp->statusCode(), drogon::k202Accepted);
}

TEST(LogE2E, post_logs_filtered_auth_success) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto writer = auth_service.create_user(
        "Writer", {Permissions::LogWrite}, "9999-12-31 23:59:59"
    );

    json body = {
        {"message", "writer log"},
        {"level", "WARN"},
        {"source", "e2e"}
    };

    auto resp = make_post_log_request(server, writer.raw_key, body);
    EXPECT_EQ(resp->statusCode(), drogon::k202Accepted);
}

TEST(LogE2E, post_logs_unauthorized) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    json body = {
        {"message", "ignored"},
        {"level", "INFO"},
        {"source", "e2e"}
    };

    auto resp = make_post_log_request(server, "", body);
    EXPECT_EQ(resp->statusCode(), drogon::k401Unauthorized);
}

TEST(LogE2E, post_logs_forbidden) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto reader = auth_service.create_user(
        "Reader", {Permissions::LogRead}, "9999-12-31 23:59:59"
    );

    json body = {
        {"message", "ignored"},
        {"level", "INFO"},
        {"source", "e2e"}
    };

    auto resp = make_post_log_request(server, reader.raw_key, body);
    EXPECT_EQ(resp->statusCode(), drogon::k403Forbidden);
}

TEST(LogE2E, post_logs_invalid_json) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer " + admin.raw_key);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody("{invalid");

    drogon::HttpResponsePtr resp;
    server.post_logs_handler(
        req,
        [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
    );

    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

TEST(LogE2E, post_logs_missing_fields) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    json body = {
        {"level", "INFO"}
    };

    auto resp = make_post_log_request(server, admin.raw_key, body);
    EXPECT_EQ(resp->statusCode(), drogon::k400BadRequest);
}

TEST(LogE2E, get_logs_no_filters) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    log_repo.insert(LogEntry{.message = "first", .level = "INFO", .source = "e2e"});
    log_repo.insert(LogEntry{.message = "second", .level = "ERROR", .source = "e2e"});

    auto resp = make_get_log_request(server, admin.raw_key);
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 2);
}

TEST(LogE2E, get_logs_filter_level) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    log_repo.insert(LogEntry{.message = "info msg", .level = "INFO", .source = "e2e"});
    log_repo.insert(LogEntry{.message = "error msg", .level = "ERROR", .source = "e2e"});

    auto resp = make_get_log_request(server, admin.raw_key, {{"level", "ERROR"}});
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 1);
    EXPECT_EQ(body[0]["level"], "ERROR");
    EXPECT_EQ(body[0]["message"], "error msg");
}

TEST(LogE2E, get_logs_filter_source) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    log_repo.insert(LogEntry{.message = "from api", .level = "INFO", .source = "api"});
    log_repo.insert(LogEntry{.message = "from cli", .level = "INFO", .source = "cli"});

    auto resp = make_get_log_request(server, admin.raw_key, {{"source", "api"}});
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 1);
    EXPECT_EQ(body[0]["source"], "api");
}

TEST(LogE2E, get_logs_filter_limit) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    for (int i = 0; i < 5; ++i) {
        log_repo.insert(LogEntry{
            .message = "log-" + std::to_string(i),
            .level = "INFO",
            .source = "e2e"
        });
    }

    auto resp = make_get_log_request(server, admin.raw_key, {{"limit", "3"}});
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 3);
}

TEST(LogE2E, get_logs_invalid_limit) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );

    log_repo.insert(LogEntry{.message = "only one", .level = "INFO", .source = "e2e"});

    auto resp = make_get_log_request(server, admin.raw_key, {{"limit", "abc"}});
    ASSERT_EQ(resp->statusCode(), drogon::k200OK);

    json body = json::parse(resp->body());
    ASSERT_EQ(body.size(), 1);
}

TEST(LogE2E, get_logs_unauthorized) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto resp = make_get_log_request(server, "");
    EXPECT_EQ(resp->statusCode(), drogon::k401Unauthorized);
}

TEST(LogE2E, get_logs_forbidden) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    auto reader = auth_service.create_user(
        "Writer", {Permissions::LogWrite}, "9999-12-31 23:59:59"
    );

    auto resp = make_get_log_request(server, reader.raw_key);
    EXPECT_EQ(resp->statusCode(), drogon::k403Forbidden);
}

