
#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <server/server.h>
#include <service/user_service.h>
#include <service/log_service.h>
#include <database/auth_repository.h>
#include <database/log_repository.h>
#include <database/database.h>


static auto prepare_post_log_request(
        SOLogSServer& server,
        const std::string& key,
        const json& body
) -> drogon::HttpRequestPtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setMethod(drogon::Post);
    req->addHeader("Authorization", "Bearer " + key);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody(body.dump());
    return req;
}

static auto prepare_get_log_request(
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

static void BM_LogE2E_post_logs_success(benchmark::State& state) {
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
    auto req = prepare_post_log_request(server, admin.raw_key, body);
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.post_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
}
BENCHMARK(BM_LogE2E_post_logs_success)
    ->Name("Sologs Post Logs Success");

static void BM_LogE2E_post_logs_unauthorized(benchmark::State& state) {
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
    auto req = prepare_post_log_request(server, "", body);
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.post_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
}
BENCHMARK(BM_LogE2E_post_logs_unauthorized)
    ->Name("Sologs Post Logs Unauthorized");

static void BM_LogE2E_post_logs_forbidden(benchmark::State& state) {
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
    auto req = prepare_post_log_request(server, reader.raw_key, body);
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.post_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
}
BENCHMARK(BM_LogE2E_post_logs_forbidden)
    ->Name("Sologs Post Logs Forbidden");

static void BM_LogE2E_post_logs_invalid_json(benchmark::State& state) {
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

    for (auto _ : state) {
        server.post_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
}
BENCHMARK(BM_LogE2E_post_logs_invalid_json)
    ->Name("Sologs Post Logs Invalid JSON");

static void BM_LogE2E_post_logs_missing_fields(benchmark::State& state) {
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
    auto req = prepare_post_log_request(server, admin.raw_key, body);
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.post_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
}
BENCHMARK(BM_LogE2E_post_logs_missing_fields)
    ->Name("Sologs Post Logs Missing JSON fields");


