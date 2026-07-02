
#include <vector>
#include <numeric>

#include <benchmark/benchmark.h>


static void BM_demo_process(benchmark::State& state) {
    std::vector<int> v(10000, 1);
    for (auto _ : state) {
        int sum = std::accumulate(v.begin(), v.end(), 0);
        benchmark::DoNotOptimize(sum);
    }
}

BENCHMARK(BM_demo_process);

BENCHMARK_MAIN();

