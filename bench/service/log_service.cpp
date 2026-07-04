
#include <memory>

#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <service/log_service.h>


class LogRepositoryMock : public ILogRepository {
    public:
        MOCK_METHOD(void, insert, (const LogEntry& entry), (override));
        MOCK_METHOD(void, insert_batch, (const std::vector<LogEntry>& entries), (override));
        MOCK_METHOD(std::vector<LogEntry>, get_all, (LogFilterParams params), (const override));
};

static std::vector<LogEntry> mock_entries(100, LogEntry{});
static std::unique_ptr<LogRepositoryMock> shared_mock_repo;
static std::unique_ptr<LogService> shared_log_service;

static void setup_log_service(const benchmark::State&) {
    shared_mock_repo = std::make_unique<LogRepositoryMock>();
    shared_log_service = std::make_unique<LogService>(*shared_mock_repo.get());
    EXPECT_CALL(*shared_mock_repo.get(), insert_batch)
        .Times(testing::AnyNumber());
}

static void teardown_log_service(const benchmark::State&) {
    shared_log_service.reset();
    shared_mock_repo.reset();
}

static void BM_LogService_create_log(benchmark::State& state) {
    json body = {
        {"message", "test log"},
        {"level", "INFO"},
        {"source", "LogService Benchmark"}
    };
    for (auto _ : state) {
        shared_log_service->create_log(body);
    }
}
BENCHMARK(BM_LogService_create_log)
    ->Setup(setup_log_service)
    ->Teardown(teardown_log_service)
    ->ThreadRange(1, std::thread::hardware_concurrency())
    ->Name("LogService::create_log: Threads");

