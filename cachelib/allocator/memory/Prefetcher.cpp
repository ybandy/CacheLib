/*
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

#include "cachelib/allocator/memory/Prefetcher.h"

namespace facebook {
namespace cachelib {

std::mutex facebook::cachelib::PrefetchTracer::traceMutex_;
std::ostream& facebook::cachelib::PrefetchTracer::traceOut_{
  !getenv("CACHELIB_TRACE_FILE") ? std::cout : *(new std::ofstream(getenv("CACHELIB_TRACE_FILE")))
};
std::atomic<uint64_t> facebook::cachelib::PrefetchTracer::counter_[facebook::cachelib::kMaxPrefetchType];

folly::ThreadLocal<std::unordered_map<int, facebook::cachelib::PrefetchStat>, facebook::cachelib::PrefetchStats::DummyPrefetchStatsTag> facebook::cachelib::PrefetchStats::tlStats;
std::unordered_map<int, facebook::cachelib::PrefetchStat> facebook::cachelib::PrefetchStats::allStats;
std::mutex facebook::cachelib::PrefetchStats::mutex;

#ifdef ENABLE_CHECKPOINT_TRACE
folly::ThreadLocal<CheckpointTrace, CheckpointTracer::DummyCheckpointTracerTag> CheckpointTracer::tlCheckpointTrace;
folly::ThreadLocal<uint32_t, CheckpointTracer::DummyCheckpointTracerTag> CheckpointTracer::tlPrefetchCounter;
#endif

} // namespace cachelib
} // namespace facebook
