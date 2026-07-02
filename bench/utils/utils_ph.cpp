
#include <benchmark/benchmark.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utils/utils.h>


static void utils_placeholder(benchmark::State& state) {
    std::vector<int> v(10000, 1);
    int sum = 0;
    for (auto _ : state) {
        for (const auto& i : v) {
            sum += i;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(utils_placeholder);

