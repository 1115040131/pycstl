// Benchmarks for the record-emitting path. Run through build.py, which builds in
// release mode -- an unoptimised build measures something else entirely:
//
//   python3 tool/build.py logger_bench
//
// A single run of a case can land 30% off its own median on a loaded machine, so read
// aggregates rather than one number:
//
//   python3 tool/build.py logger_bench -- \
//       --benchmark_repetitions=8 --benchmark_report_aggregates_only=true
//
// and treat a case whose reported cv exceeds a few percent as noise rather than signal.

#include <benchmark/benchmark.h>

#include <cstdio>
#include <memory>

#include "logger/logger.h"
#include "logger/sink.h"

namespace pyc {
namespace {

// NullSink measures formatting on its own: it holds no state and therefore takes no
// lock, so what remains is the record being built. FileSink writes to /dev/null to add
// the cost of the write without involving a terminal or the file system.
std::shared_ptr<LogSink> DevNullSink() {
    static const std::shared_ptr<LogSink> sink = [] -> std::shared_ptr<LogSink> {
        std::FILE* file = std::fopen("/dev/null", "w");
        if (file == nullptr) {
            std::perror("logger_benchmark: /dev/null");
            std::abort();
        }
        return std::make_shared<FileSink>(file);
    }();
    return sink;
}

// Arguments vary per iteration: with compile-time constants the optimiser can fold away
// conversion work that a real caller pays for.
void BM_NoArgs(benchmark::State& state) {
    Logger logger("BENCH", std::make_shared<NullSink>());
    for (auto _ : state) {
        logger.info("a message with no arguments");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_NoArgs);

void BM_MixedArgs(benchmark::State& state) {
    Logger logger("BENCH", std::make_shared<NullSink>());
    int i = 0;
    for (auto _ : state) {
        logger.info("formatted int: {}, float: {}, string: {}", i++, 2.72, "hello");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MixedArgs);

// Should cost close to nothing: the level is checked before the record is formatted, so
// only argument evaluation survives.
void BM_FilteredByLogger(benchmark::State& state) {
    Logger logger("BENCH", std::make_shared<NullSink>());
    logger.set_level(LogLevel::kError);
    int i = 0;
    for (auto _ : state) {
        logger.debug("formatted int: {}, float: {}, string: {}", i++, 2.72, "hello");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_FilteredByLogger);

// The record is still rendered here -- only the sink rejects it -- so the difference
// from BM_MixedArgs is what a sink-level filter actually saves.
void BM_FilteredBySink(benchmark::State& state) {
    auto sink = std::make_shared<NullSink>();
    sink->set_level(LogLevel::kError);
    Logger logger("BENCH", sink);
    int i = 0;
    for (auto _ : state) {
        logger.debug("formatted int: {}, float: {}, string: {}", i++, 2.72, "hello");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_FilteredBySink);

void BM_WriteToDevNull(benchmark::State& state) {
    Logger logger("BENCH", DevNullSink());
    int i = 0;
    for (auto _ : state) {
        logger.info("formatted int: {}, float: {}, string: {}", i++, 2.72, "hello");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_WriteToDevNull);

void BM_FanOutToTwoSinks(benchmark::State& state) {
    Logger logger("BENCH", {std::make_shared<NullSink>(), std::make_shared<NullSink>()});
    int i = 0;
    for (auto _ : state) {
        logger.info("formatted int: {}, float: {}, string: {}", i++, 2.72, "hello");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_FanOutToTwoSinks);

// One logger and one sink shared by every thread. NullSink takes no lock, so this shows
// how far a record gets on its own; the FileSink variant below adds the sink's mutex.
Logger& SharedNullLogger() {
    static Logger logger("BENCH", std::make_shared<NullSink>());
    return logger;
}

void BM_ThreadedFormatting(benchmark::State& state) {
    Logger& logger = SharedNullLogger();
    int i = 0;
    for (auto _ : state) {
        logger.info("formatted int: {}, float: {}, string: {}", i++, 2.72, "hello");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ThreadedFormatting)->ThreadRange(1, 8)->UseRealTime();

Logger& SharedFileLogger() {
    static Logger logger("BENCH", DevNullSink());
    return logger;
}

void BM_ThreadedWrite(benchmark::State& state) {
    Logger& logger = SharedFileLogger();
    int i = 0;
    for (auto _ : state) {
        logger.info("formatted int: {}, float: {}, string: {}", i++, 2.72, "hello");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ThreadedWrite)->ThreadRange(1, 8)->UseRealTime();

}  // namespace
}  // namespace pyc

BENCHMARK_MAIN();
