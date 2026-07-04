
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

static auto prepare_get_auth_request(
        SOLogSServer& server,
        const std::string& key,
        const std::vector<std::pair<std::string, std::string>>& params = {}
) -> drogon::HttpRequestPtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/auth");
    req->setMethod(drogon::Get);
    if (!key.empty())
        req->addHeader("Authorization", "Bearer " + key);
    for (const auto& [k, v] : params)
        req->setParameter(k, v);
    return req;
}

static void BM_AuthE2E_get_auth(benchmark::State& state) {
    SQLiteDatabase db(":memory:");
    SqlAuthRepository auth_repo(db);
    UserService auth_service(auth_repo);
    LogServiceMock log_mock;
    SOLogSServer server(log_mock, auth_service);

    for (size_t i = 0; i < state.range(0); i++) {
        (void)auth_service.create_user(
            "Bob", {Permissions::LogRead}, "9999-12-31 23:59:59"
        );
    }

    auto admin = auth_service.create_user(
        "Alice", {Permissions::Admin}, "9999-12-31 23:59:59"
    );
    auto req = prepare_get_auth_request(
            server,
            admin.raw_key,
            {{"limit", std::to_string(state.range(0))}}
    );
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.get_auth_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
}
BENCHMARK(BM_AuthE2E_get_auth)
    ->Name("sologs GET /auth: Limits")
    ->RangeMultiplier(2)
    ->Range(32, 1024);

