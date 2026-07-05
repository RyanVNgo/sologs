
#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <server/server.h>
#include <service/user_service.h>
#include <service/log_service.h>
#include <database/auth_repository.h>
#include <database/log_repository.h>
#include <database/database.h>


static auto prepare_get_log_request(
        SOLogSServer& server,
        const std::string& key,
        const std::vector<std::pair<std::string, std::string>>& params = {}
) -> drogon::HttpRequestPtr {
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath("/logs");
    req->setMethod(drogon::Get);
    req->addHeader("Authorization", "Bearer " + key);
    for (const auto& [k, v] : params)
        req->setParameter(k, v);
    return req;
}

static void BM_LogE2E_get_logs(benchmark::State& state) {
    SQLiteDatabase log_db(":memory:");
    SQLiteDatabase auth_db(":memory:");
    SqlLogRepository log_repo(log_db);
    SqlAuthRepository auth_repo(auth_db);
    LogService log_service(log_repo);
    UserService auth_service(auth_repo);
    SOLogSServer server(log_service, auth_service);

    for (size_t i = 0; i < state.range(0); i++) {
        log_repo.insert(
                LogEntry{
                    .message = "example message for a log",
                    .level = "INFO",
                    .source = "end-to-end benchmarking"
                }
        );
    }

    auto admin = auth_service.create_user(
        "Admin", {Permissions::Admin}, "9999-12-31 23:59:59"
    );
    auto req = prepare_get_log_request(
            server,
            admin.raw_key,
            {{"limit", std::to_string(state.range(0))}}
    );
    drogon::HttpResponsePtr resp;

    for (auto _ : state) {
        server.get_logs_handler(
            req,
            [&resp](const drogon::HttpResponsePtr& r) { resp = r; }
        );
    }
}
BENCHMARK(BM_LogE2E_get_logs)
    ->Name("sologs GET /logs: Limits")
    ->RangeMultiplier(2)
    ->Range(32, 1024);

