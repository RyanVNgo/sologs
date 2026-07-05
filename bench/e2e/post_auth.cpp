
#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <server/server.h>
#include <service/user_service.h>
#include <service/log_service.h>
#include <database/auth_repository.h>
#include <database/log_repository.h>
#include <database/database.h>


class LogServiceMock : public ILogService {
    public:
        MOCK_METHOD(void, create_log, (const json& body), (override));
        MOCK_METHOD(json, get_logs, (LogFilterParams params), (const override));
};

static auto prepare_post_auth_request(
        SOLogSServer& server,
        const std::string& key,
        const json& body
) -> drogon::HttpRequestPtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setMethod(drogon::Post);
    if (!key.empty())
        req->addHeader("Authorization", "Bearer " + key);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody(body.dump());
    return req;
}

static void BM_AuthE2E_post_auth(benchmark::State& state) {
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
    auto req = prepare_post_auth_request(server, admin.raw_key, body);
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.post_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
};
BENCHMARK(BM_AuthE2E_post_auth)
    ->Name("sologs POST /auth: OK");

static void BM_AuthE2E_post_auth_invalid_json(benchmark::State& state) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );
    auto req = prepare_post_auth_request(server, admin.raw_key, "");
    req->setBody("{invalid json");
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.post_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
};
BENCHMARK(BM_AuthE2E_post_auth_invalid_json)
    ->Name("sologs POST /auth: Invalid JSON");

static void BM_AuthE2E_post_auth_missing_field(benchmark::State& state) {
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
    };
    auto req = prepare_post_auth_request(server, admin.raw_key, body);
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.post_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
};
BENCHMARK(BM_AuthE2E_post_auth_missing_field)
    ->Name("sologs POST /auth: Missing JSON Field");

