#include "benchmark/benchmark.h"

#include "soro/parse_train_data.h"
#include "soro/running_time_calculation.h"

using namespace soro;

auto const info = parse_train_data(R"(PASTE HERE)");

static void acceleration_numerical(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(compute_train_run(info.front(), 160));
  }
}
BENCHMARK(acceleration_numerical)->Threads(8);

static void acceleration_analytical(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(compute_train_run(info.front(), 160));
  }
}
BENCHMARK(acceleration_analytical)->Threads(8);

BENCHMARK_MAIN();