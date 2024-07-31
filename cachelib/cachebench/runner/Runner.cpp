/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * Copyright (c) 2024 Kioxia Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/resource.h>

#include "cachelib/allocator/memory/Prefetcher.h"

#include "cachelib/cachebench/runner/Runner.h"

#include "cachelib/cachebench/runner/Stressor.h"

namespace facebook {
namespace cachelib {
namespace cachebench {
Runner::Runner(const CacheBenchConfig& config)
    : stressor_{Stressor::makeStressor(config.getCacheConfig(),
                                       config.getStressorConfig())} {}

bool Runner::run(std::chrono::seconds progressInterval,
                 const std::string& progressStatsFile) {
  ProgressTracker tracker{*stressor_, progressStatsFile};
  struct rusage rUsageBefore = {};
  struct rusage rUsageAfter = {};

  if (::getrusage(RUSAGE_SELF, &rUsageBefore)) {
    throw std::runtime_error("Error getting rusage");
  }
  stressor_->start();

  if (!tracker.start(progressInterval)) {
    throw std::runtime_error("Cannot start ProgressTracker.");
  }

  stressor_->finish();
  if (::getrusage(RUSAGE_SELF, &rUsageAfter)) {
    throw std::runtime_error("Error getting rusage");
  }

  uint64_t durationNs = stressor_->getTestDurationNs();
  auto cacheStats = stressor_->getCacheStats();
  auto opsStats = stressor_->aggregateThroughputStats();
  tracker.stop();

  std::cout << "== Test Results ==\n== Allocator Stats ==" << std::endl;
  cacheStats.render(std::cout);

  std::cout << "\n== Throughput for  ==\n";
  opsStats.render(durationNs, std::cout);

  std::cout << "\n== CPU utilization ==\n";
  auto getTimeRUsage = [](const struct rusage& r) {
    double userSeconds = r.ru_utime.tv_sec + r.ru_utime.tv_usec / 1e6;
    double sysSeconds = r.ru_stime.tv_sec + r.ru_stime.tv_usec / 1e6;
    return std::make_tuple(userSeconds, sysSeconds, userSeconds + sysSeconds);
  };

  auto [beforeUser, beforeSys, beforeTot] = getTimeRUsage(rUsageBefore);
  auto [afterUser, afterSys, afterTot] = getTimeRUsage(rUsageAfter);

  std::cout << folly::sformat(
      "user: {:3.6}s, sys: {:3.6f}s, total: {:3.6f}s, util-pct: {:2.2f}%\n",
      afterUser - beforeUser, afterSys - beforeSys, afterTot - beforeTot,
      100.0 * (afterTot - beforeTot) / (durationNs / 1000000000));

  std::cout << "\n== Prefetch ==\n";
  facebook::cachelib::PrefetchTracer::flushTrace();
  facebook::cachelib::PrefetchTracer::printCounter();
  facebook::cachelib::PrefetchStats::render();

  std::cout << "\n== Mutex Stats ==\n";
  facebook::cachelib::MutexStats::render();

  stressor_->renderWorkloadGeneratorStats(durationNs, std::cout);
  std::cout << std::endl;

  stressor_.reset();
  return cacheStats.renderIsTestPassed(std::cout);
}

void Runner::run(folly::UserCounters& counters) {
  stressor_->start();
  stressor_->finish();

  BENCHMARK_SUSPEND {
    uint64_t durationNs = stressor_->getTestDurationNs();
    auto cacheStats = stressor_->getCacheStats();
    auto opsStats = stressor_->aggregateThroughputStats();

    // Allocator Stats
    cacheStats.render(counters);

    // Throughput
    opsStats.render(durationNs, counters);

    stressor_->renderWorkloadGeneratorStats(durationNs, counters);

    counters["nvm_disable"] = cacheStats.isNvmCacheDisabled ? 100 : 0;
    counters["inconsistency_count"] = cacheStats.inconsistencyCount * 100;

    stressor_.reset();
  }
}

} // namespace cachebench
} // namespace cachelib
} // namespace facebook
