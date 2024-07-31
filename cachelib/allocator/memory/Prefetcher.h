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

#pragma once

#include <atomic>
#include <fstream>
#include <iostream>
#include <mutex>
#include <folly/fibers/FiberManager.h>
#include <folly/logging/xlog.h>
#include <folly/Format.h>

#include "cachelib/allocator/memory/Slab.h"
#include "cachelib/common/Time.h"

namespace facebook {
namespace cachelib {

enum PrefetchType {
    kAcquire,
    kAccessContainerKeyInsert,
    kAccessContainerKeyInsertOrReplace,
    kAccessContainerKeyReplaceIf,
    kAccessContainerKeyRemove,
    kAccessContainerKeyRemoveIf,
    kAccessContainerInsertOrReplace,
    kAccessContainerFind,
    kAccessContainerFindPrev,
    kMMContainerRecordAccess,
    kMMContainerLinkAtHead,
    kMMContainerInsertBefore,
    kMMContainerUpdateInsertionPoint,
    kMMContainerUnlink,
    kRelease,
    kPreamble,
    kEvictionIterator,
    kChainedItem,
    kFreeListPop,
    kFreeListPush,
    kReaper,
    kKey,
    kReadValue,
    kWriteValue,
    kMove,
    kMisc,
    kMaxPrefetchType,
};

class PrefetchTracer {
 public:

  static void flushTrace() {
    traceOut_.flush();
  }

  static void printCounter() {
    uint64_t total = 0;

    for (int i = 0; i < kMaxPrefetchType; i++) {
      total += counter_[i].load();
      std::cout << folly::sformat( "{} : {}\n", prefetchTypeName[i], counter_[i].load());
    }
    std::cout << folly::sformat( "total : {}\n", total);
  }

  static std::atomic<uint64_t> counter_[kMaxPrefetchType];

  struct TraceRecord {
    uint64_t address;
    uint64_t ip;
    uint32_t size;
    uint32_t delay;
    uint32_t type;
  };

  static void trace(const void *memory, size_t len, long delay, enum PrefetchType type, void *ip) {
    TraceRecord record {
      reinterpret_cast<uint64_t>(memory),
      reinterpret_cast<uint64_t>(ip),
      static_cast<uint32_t>(len),
      static_cast<uint32_t>(delay),
      static_cast<uint32_t>(type)
    };

    std::lock_guard<std::mutex> lock(traceMutex_);
    traceOut_ << record.address << ",";
    traceOut_ << record.ip << ",";
    traceOut_ << record.size << ",";
    traceOut_ << record.delay<< ",";
    traceOut_ << record.type << std::endl;
  }

  static constexpr const char *prefetchTypeName[kMaxPrefetchType] = {
    [kAcquire] = "acquire",
    [kAccessContainerKeyInsert] = "accessContainerKeyInsert",
    [kAccessContainerKeyInsertOrReplace] = "accessContainerKeyInsertOrReplace",
    [kAccessContainerKeyReplaceIf] = "accessContainerKeyReplaceIf",
    [kAccessContainerKeyRemove] = "accessContainerKeyRemove",
    [kAccessContainerKeyRemoveIf] = "accessContainerKeyRemoveIf",
    [kAccessContainerInsertOrReplace] = "accessContainerInsertOrReplace",
    [kAccessContainerFind] = "accessContainerFind",
    [kAccessContainerFindPrev] = "accessContainerFindPrev",
    [kMMContainerRecordAccess] = "mmContainerRecordAccess",
    [kMMContainerLinkAtHead] = "mmContainerLinkAtHead",
    [kMMContainerInsertBefore] = "mmContainerInsertBefore",
    [kMMContainerUpdateInsertionPoint] = "mmContainerUpdateInsertionPoint",
    [kMMContainerUnlink] = "mmContainerUnlink",
    [kRelease] = "release",
    [kPreamble] = "preamble",
    [kEvictionIterator] = "evictionIterator",
    [kChainedItem] = "chainedItem",
    [kFreeListPop] = "freeListPop",
    [kFreeListPush] = "freeListPush",
    [kReaper] = "reaper",
    [kKey] = "key",
    [kReadValue] = "readValue",
    [kWriteValue] = "writeValue",
    [kMove] = "move",
    [kMisc] = "misc"
  };

 private:

  static std::mutex traceMutex_;
  static std::ostream& traceOut_;

};

//#define PREFETCH_STATS

struct PrefetchStat {
  uint64_t prefetch_count{0};
  uint64_t total_yield_time{0};
  uint64_t min_yield_time{UINT64_MAX};
  uint64_t max_yield_time{0};
};

class PrefetchStats {
 public:
  class DummyPrefetchStatsTag {};
  static folly::ThreadLocal<std::unordered_map<int, PrefetchStat>, DummyPrefetchStatsTag> tlStats;
  static std::unordered_map<int, PrefetchStat> allStats;
  static std::mutex mutex;

  static void record(enum PrefetchType type, uint64_t time) {
    if (PrefetchStats::tlStats->find(type) == PrefetchStats::tlStats->end()) {
      PrefetchStats::tlStats->emplace(type, PrefetchStat{});
    }
    auto& stat = PrefetchStats::tlStats->at(type);
    stat.prefetch_count++;
    stat.total_yield_time += time;
    stat.min_yield_time = std::min(stat.min_yield_time, time);
    stat.max_yield_time = std::max(stat.max_yield_time, time);
  }

  static void flush() {
    std::lock_guard<std::mutex> l(mutex);
    for (auto& stat: *tlStats) {
      if (allStats.find(stat.first) == allStats.end()) {
        allStats.emplace(stat.first, stat.second);
      } else {
        auto& allStat = allStats.at(stat.first);
        allStat.prefetch_count += stat.second.prefetch_count;
        allStat.total_yield_time += stat.second.total_yield_time;
        allStat.min_yield_time = std::min(allStat.min_yield_time, stat.second.min_yield_time);
        allStat.max_yield_time = std::max(allStat.max_yield_time, stat.second.max_yield_time);
      }
      stat.second.prefetch_count = 0;
      stat.second.total_yield_time = 0;
      stat.second.min_yield_time = UINT64_MAX;
      stat.second.max_yield_time = 0;
    }
  }

  static void render() {
    std::cout << "#type,prefetch_count,total_yield_time,min_yield_time,max_yield_time\n";
    for (auto& stat: allStats) {
      std::cout << folly::sformat("{},{},{},{},{}\n",
          PrefetchTracer::prefetchTypeName[stat.first], stat.second.prefetch_count, stat.second.total_yield_time,
          stat.second.min_yield_time, stat.second.max_yield_time);
    }
  }
};

#undef ENABLE_CHECKPOINT_TRACE

enum checkpoint_trace_type {
  kReadSubmitBegin = 2 * (kMaxPrefetchType + 1),
  kReadSubmitEnd,
  kWriteSubmitBegin,
  kWriteSubmitEnd,
  kReadBegin,
  kReadEnd,
  kWriteBegin,
  kWriteEnd,
  kPrefetchYieldBegin,
  kPrefetchYieldEnd,
  kReadYieldBegin,
  kReadYieldEnd,
  kWriteYieldBegin,
  kWriteYieldEnd,
  kSetOpBegin,
  kSetOpEnd,
  kGetOpBegin,
  kGetOpEnd,
  kDelOpBegin,
  kDelOpEnd,
  kBenchmarkBegin,
  kBenchmarkEnd,
  kMaxCheckpoint,
};

#ifdef ENABLE_CHECKPOINT_TRACE

// copied from liburing/src/include/liburing/barrier.h

template <typename T>
static inline void WRITE_ONCE(T &var, T val)
{
  std::atomic_store_explicit(reinterpret_cast<std::atomic<T> *>(&var),
                             val, std::memory_order_relaxed);
}
template <typename T>
static inline T READ_ONCE(const T &var)
{
  return std::atomic_load_explicit(reinterpret_cast<const std::atomic<T> *>(&var),
                                   std::memory_order_relaxed);
}

template <typename T>
static inline void smp_store_release(T *p, T v)
{
  std::atomic_store_explicit(reinterpret_cast<std::atomic<T> *>(p), v,
                             std::memory_order_release);
}

template <typename T>
static inline T smp_load_acquire(const T *p)
{
  return std::atomic_load_explicit(reinterpret_cast<const std::atomic<T> *>(p),
                                   std::memory_order_acquire);
}

static inline void smp_mb()
{
  std::atomic_thread_fence(std::memory_order_seq_cst);
}

struct CheckpointTraceEntry {
  uint64_t id;
  uint32_t checkpoint;
  uint32_t prefetchCounter;
  uint64_t timestamp;
};

struct CheckpointTrace {
  uint64_t capacity;
  uint64_t len;
  struct CheckpointTraceEntry *buffer;
};

class CheckpointTracer {
 public:
  static void dump(std::ostream& out) {
    bool empty = true;
    int pid = 0;

    out << "[" << std::endl;

    for (auto& tl : tlCheckpointTrace.accessAllThreads()) {
      uint64_t len = smp_load_acquire(&tl.len);

      for (uint64_t i = 0; i < len; i++) {
        uint32_t checkpoint = tl.buffer[i].checkpoint;

        assert(checkpoint < kMaxCheckpoint);

        if (i == 0 && !empty) {
          out << "," << std::endl;
        }
        empty = false;

        out << "\t{" << std::endl;
        out << "\t\t\"name\" : \""<< checkpointName(checkpoint) << "\"," << std::endl;
        out << "\t\t\"ph\" : \""<< checkpointPhase(checkpoint) << "\"," << std::endl;
        out << "\t\t\"pid\" : "<< pid << "," << std::endl;
        out << "\t\t\"tid\" : "<< tl.buffer[i].id << "," << std::endl;
        out << "\t\t\"ts\" : "<< tl.buffer[i].timestamp << "," << std::endl;
        out << "\t\t\"args\" : { \"prefetchCounter\" : " << tl.buffer[i].prefetchCounter << " }" << std::endl;
        if (i < len - 1) {
          out << "\t}," << std::endl;
        } else {
          out << "\t}" << std::endl;
        }
      }
      pid++;
    }

    out << "]" << std::endl;
  }

  static void reset() {
    for (auto& tl : tlCheckpointTrace.accessAllThreads()) {
      const uint64_t zero = 0;
      WRITE_ONCE(tl.len, zero);
    }
  }

#undef ENABLE_CHECKPOINT_TRACE_BENCHMARK
#undef ENABLE_CHECKPOINT_TRACE_REDUCED

  static void record(uint32_t checkpoint) {
#ifdef ENABLE_CHECKPOINT_TRACE_BENCHMARK
    uint64_t toggle = 0;
benchmark:
    checkpoint = kBenchmarkBegin + toggle;
#endif

#ifdef ENABLE_CHECKPOINT_TRACE_REDUCED
    if (checkpoint < kMaxPrefetchType * 2) {
      (*tlPrefetchCounter)++;
    }
    /* trace only yield, op, and benchmark checkpoints */
    if (checkpoint < kPrefetchYieldBegin) {
      return;
    }
#endif
    if (tlCheckpointTrace->capacity == 0) {
      const uint64_t zero = 0;
      tlCheckpointTrace->capacity = kDefaultCapacity;
      WRITE_ONCE(tlCheckpointTrace->len, zero);
      tlCheckpointTrace->buffer = new CheckpointTraceEntry[tlCheckpointTrace->capacity];
    }

    uint64_t len = smp_load_acquire(&tlCheckpointTrace->len);

    if (len < tlCheckpointTrace->capacity) {
      auto fm = folly::fibers::FiberManager::getFiberManagerUnsafe();
      if (fm) {
        tlCheckpointTrace->buffer[len].id = (uint64_t)fm->currentFiber();
      } else {
        tlCheckpointTrace->buffer[len].id = 0;
      }
      tlCheckpointTrace->buffer[len].checkpoint = checkpoint;
#ifdef ENABLE_CHECKPOINT_TRACE_REDUCED
      tlCheckpointTrace->buffer[len].prefetchCounter = *tlPrefetchCounter;
#else
      tlCheckpointTrace->buffer[len].prefetchCounter = 0;
#endif
      tlCheckpointTrace->buffer[len].timestamp = util::getCurrentTimeNs();
      smp_store_release(&tlCheckpointTrace->len, len + 1);
    }
#ifdef ENABLE_CHECKPOINT_TRACE_BENCHMARK
    if (len < tlCheckpointTrace->capacity) {
      toggle ^= 1;
      goto benchmark;
    }
#endif
  }

 private:
  static constexpr uint64_t kDefaultCapacity = 128 * 1024;
  class DummyCheckpointTracerTag {};
  static folly::ThreadLocal<CheckpointTrace, DummyCheckpointTracerTag> tlCheckpointTrace;
  static folly::ThreadLocal<uint32_t, DummyCheckpointTracerTag> tlPrefetchCounter;

  static const char* checkpointName(uint32_t checkpoint) {
    if (checkpoint / 2 < kMaxPrefetchType) {
      return PrefetchTracer::prefetchTypeName[checkpoint / 2];
    }
    switch (checkpoint) {
    case kPrefetchYieldBegin:
    case kPrefetchYieldEnd:
      return "prefetch_yield";
    case kReadSubmitBegin:
    case kReadSubmitEnd:
      return "read_submit";
    case kReadYieldBegin:
    case kReadYieldEnd:
      return "read_yield";
    case kReadBegin:
    case kReadEnd:
      return "read";
    case kWriteSubmitBegin:
    case kWriteSubmitEnd:
      return "write_submit";
    case kWriteYieldBegin:
    case kWriteYieldEnd:
      return "write_yield";
    case kWriteBegin:
    case kWriteEnd:
      return "write";
    case kSetOpBegin:
    case kSetOpEnd:
      return "set_op";
    case kGetOpBegin:
    case kGetOpEnd:
      return "get_op";
    case kDelOpBegin:
    case kDelOpEnd:
      return "del_op";
    case kBenchmarkBegin:
    case kBenchmarkEnd:
      return "benchmark";
    default:
      break;
    }
    return NULL;
  }

  static const char* checkpointPhase(uint32_t checkpoint) {
    return (checkpoint % 2) == 0 ? "B" : "E";
  }
};

#else /* ENABLE_CHECKPOINT_TRACE */

class CheckpointTracer {
 public:
  constexpr static inline void dump(std::ostream& out) { }
  constexpr static inline void reset() { }
  constexpr static inline void record(uint32_t checkpoint) { }
};

#endif /* ENABLE_CHECKPOINT_TRACE */

struct PrefetchInfo {
  // 0: disalbe, 1: enable, 2: enable but no yield
  int enabled;
};

#define STORE_PREFETCH 1

static constexpr struct PrefetchInfo prefetchInfo[kMaxPrefetchType] = {
    [kAcquire] = { STORE_PREFETCH },
    [kAccessContainerKeyInsert] = { 1 },
    [kAccessContainerKeyInsertOrReplace] = { 0 },
    [kAccessContainerKeyReplaceIf] = { 1 },
    [kAccessContainerKeyRemove] = { 1 },
    [kAccessContainerKeyRemoveIf] = { 1 },
    [kAccessContainerInsertOrReplace] = { 1 },
    [kAccessContainerFind] = { 1 },
    [kAccessContainerFindPrev] = { 0 },
    [kMMContainerRecordAccess] = { 0 },
    [kMMContainerLinkAtHead] = { STORE_PREFETCH },
    [kMMContainerInsertBefore] = { STORE_PREFETCH },
    [kMMContainerUpdateInsertionPoint] = { 1 },
    [kMMContainerUnlink] = { STORE_PREFETCH },
    [kRelease] = { 0 },
    [kPreamble] = { 0 },
    [kEvictionIterator] = { 1 },
    [kChainedItem] = { 1 },
    [kFreeListPop] = { 1 },
    [kFreeListPush] = { 0 },
    [kReaper] = { 0 },
    [kKey] = { 1 },
    [kReadValue] = { 1 },
    [kWriteValue] = { STORE_PREFETCH },
    [kMove] = { STORE_PREFETCH },
    [kMisc] = { 1 },
};

template <typename AllocatorT>
class Prefetcher {
 public:
  explicit Prefetcher(const AllocatorT& allocator, int delay) noexcept
      : allocator_(allocator), delay_{delay} {}

#define ENABLE_COUNTER 0
#define COUNT_ZERO_DELAY 0
#define ENABLE_TRACE 0
#define TRACE_ZERO_DELAY 0
#undef EMULATE_PERFETCH_DELAY

  void access(const void* memory, size_t len = 64, bool do_yield = 1) const {
    if (ENABLE_COUNTER)
      PrefetchTracer::counter_[kMisc]++;
    if (ENABLE_TRACE)
      PrefetchTracer::trace(memory, len, delay_.count(), kMisc, __builtin_return_address(0));
    raw_access(kMisc, memory, len, do_yield);
  }

#define DEFINE_ACCESS(type)	\
  void access##type(const void* memory, size_t len = 64, int do_yield = -1) const {	\
    if (memory != nullptr && prefetchInfo[k##type].enabled)	{	\
      if ((COUNT_ZERO_DELAY || (delay_.count() > 0)) && ENABLE_COUNTER)	\
        PrefetchTracer::counter_[k##type]++;	\
      if ((TRACE_ZERO_DELAY || (delay_.count() > 0)) && ENABLE_TRACE)	\
        PrefetchTracer::trace(memory, len, delay_.count(), k##type, __builtin_return_address(0));	\
      raw_access(k##type, memory, len, do_yield == -1 ? prefetchInfo[k##type].enabled == 1 : do_yield);	\
    }	\
  }

  DEFINE_ACCESS(Acquire);
  DEFINE_ACCESS(AccessContainerKeyInsert);
  DEFINE_ACCESS(AccessContainerKeyInsertOrReplace);
  DEFINE_ACCESS(AccessContainerKeyReplaceIf);
  DEFINE_ACCESS(AccessContainerKeyRemove);
  DEFINE_ACCESS(AccessContainerKeyRemoveIf);
  DEFINE_ACCESS(AccessContainerInsertOrReplace);
  DEFINE_ACCESS(AccessContainerFind);
  DEFINE_ACCESS(AccessContainerFindPrev);
  DEFINE_ACCESS(MMContainerRecordAccess);
  DEFINE_ACCESS(MMContainerLinkAtHead);
  DEFINE_ACCESS(MMContainerInsertBefore);
  DEFINE_ACCESS(MMContainerUpdateInsertionPoint);
  DEFINE_ACCESS(MMContainerUnlink);
  DEFINE_ACCESS(Release);
  DEFINE_ACCESS(Preamble);
  DEFINE_ACCESS(EvictionIterator);
  DEFINE_ACCESS(ChainedItem);
  DEFINE_ACCESS(FreeListPop);
  DEFINE_ACCESS(FreeListPush);
  DEFINE_ACCESS(Reaper);
  DEFINE_ACCESS(Key);
  DEFINE_ACCESS(ReadValue);
  DEFINE_ACCESS(WriteValue);
  DEFINE_ACCESS(Move);

 private:

#define cpu_relax() asm volatile("pause\n": : :"memory")

  void raw_access(enum PrefetchType type, const void* memory, size_t len, bool do_yield) const {
#if 0
    if (!allocator_.getSlabHeader(memory)) {
      return;
    }
#endif
    if (!delay_.count()) {
      return;
    }
#ifdef EMULATE_PERFETCH_DELAY
    std::chrono::time_point<std::chrono::steady_clock> before_yield = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> end = before_yield + delay_;
    folly::fibers::yield();
#ifdef PREFETCH_STATS
    std::chrono::time_point<std::chrono::steady_clock> after_yield = std::chrono::steady_clock::now();
    std::chrono::duration time = after_yield - before_yield;
    PrefetchStats::record(type, time.count());
#endif
    while (std::chrono::steady_clock::now() < end) {
      cpu_relax();
    }
#else
    intptr_t memory_aligned0 = intptr_t(memory) & (~intptr_t(64 - 1));
    intptr_t memory_aligned1 = intptr_t(memory + len - 1) & (~intptr_t(64 - 1));
    for(; memory_aligned0 <= memory_aligned1; memory_aligned0 += 64){
      CheckpointTracer::record(2 * type + 0 /* begin */);
      __builtin_prefetch((void*)memory_aligned0);
      CheckpointTracer::record(2 * type + 1 /* end */);
    }

    if (do_yield) {
      CheckpointTracer::record(kPrefetchYieldBegin);
      folly::fibers::yield();
      CheckpointTracer::record(kPrefetchYieldEnd);
    }
#endif
  }

  const AllocatorT& allocator_;
  const std::chrono::nanoseconds delay_{0};
};

template <typename AllocatorContainer>
class MultiTierPrefetcher {
 public:
  explicit MultiTierPrefetcher(const AllocatorContainer& allocators, int delay) noexcept
      : allocators_(allocators), delay_{delay} {}

  DEFINE_ACCESS(Acquire);
  DEFINE_ACCESS(AccessContainerKeyInsert);
  DEFINE_ACCESS(AccessContainerKeyInsertOrReplace);
  DEFINE_ACCESS(AccessContainerKeyReplaceIf);
  DEFINE_ACCESS(AccessContainerKeyRemove);
  DEFINE_ACCESS(AccessContainerKeyRemoveIf);
  DEFINE_ACCESS(AccessContainerInsertOrReplace);
  DEFINE_ACCESS(AccessContainerFind);
  DEFINE_ACCESS(AccessContainerFindPrev);
  DEFINE_ACCESS(MMContainerRecordAccess);
  DEFINE_ACCESS(MMContainerLinkAtHead);
  DEFINE_ACCESS(MMContainerInsertBefore);
  DEFINE_ACCESS(MMContainerUpdateInsertionPoint);
  DEFINE_ACCESS(MMContainerUnlink);
  DEFINE_ACCESS(Release);
  DEFINE_ACCESS(Preamble);
  DEFINE_ACCESS(EvictionIterator);
  DEFINE_ACCESS(ChainedItem);
  DEFINE_ACCESS(FreeListPop);
  DEFINE_ACCESS(FreeListPush);
  DEFINE_ACCESS(Reaper);
  DEFINE_ACCESS(Key);
  DEFINE_ACCESS(ReadValue);
  DEFINE_ACCESS(WriteValue);
  DEFINE_ACCESS(Move);

 private:

  void raw_access(enum PrefetchType type, const void* memory, size_t len, bool do_yield) const {
    if (!delay_.count()) {
      return;
    }
    TierId tid;
    for (tid = 0; tid < allocators_.size(); tid++) {
      if (allocators_[tid]->isMemoryInAllocator(memory))
        break;
    }
    if (tid == 0 && allocators_.size() > 1) {
      return;
    }
    if (tid == allocators_.size()) {
      XLOGF(ERR, "Invalid prefetch address with type {}", PrefetchTracer::prefetchTypeName[type]);
    }
#ifdef EMULATE_PERFETCH_DELAY
    std::chrono::time_point<std::chrono::steady_clock> before_yield = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> end = before_yield + delay_;
    folly::fibers::yield();
#ifdef PREFETCH_STATS
    std::chrono::time_point<std::chrono::steady_clock> after_yield = std::chrono::steady_clock::now();
    std::chrono::duration time = after_yield - before_yield;
    PrefetchStats::record(type, time.count());
#endif
    while (std::chrono::steady_clock::now() < end) {
      cpu_relax();
    }
#else
    intptr_t memory_aligned0 = intptr_t(memory) & (~intptr_t(64 - 1));
    intptr_t memory_aligned1 = intptr_t(memory + len - 1) & (~intptr_t(64 - 1));
    for(; memory_aligned0 <= memory_aligned1; memory_aligned0 += 64){
      CheckpointTracer::record(2 * type + 0 /* begin */);
      __builtin_prefetch((void*)memory_aligned0);
      CheckpointTracer::record(2 * type + 1 /* end */);
    }

    if (do_yield) {
      CheckpointTracer::record(kPrefetchYieldBegin);
      folly::fibers::yield();
      CheckpointTracer::record(kPrefetchYieldEnd);
    }
#endif
  }

  const AllocatorContainer& allocators_;
  const std::chrono::nanoseconds delay_{0};
};

} // namespace cachelib
} // namespace facebook
