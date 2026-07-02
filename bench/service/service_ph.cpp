
#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <service/log_service.h>
#include <service/user_service.h>


static void service_placeholder(benchmark::State& state) {
    std::vector<int> v(10000, 1);
    int sum = 0;
    for (auto _ : state) {
        for (const auto& i : v) {
            sum += i;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(service_placeholder);

