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

#include "cachelib/common/Mutex.h"

folly::ThreadLocal<std::unordered_map<void*, facebook::cachelib::MutexStat>, facebook::cachelib::MutexStats::DummyMutexStatsTag> facebook::cachelib::MutexStats::tlStats;
std::unordered_map<void*, facebook::cachelib::MutexStat> facebook::cachelib::MutexStats::allStats;
std::mutex facebook::cachelib::MutexStats::mutex;
