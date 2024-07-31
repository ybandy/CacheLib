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

#include "cachelib/cachebench/workload/WorkloadGenerator.h"

#include <algorithm>
#include <chrono>
#include <iostream>
namespace facebook {
namespace cachelib {
namespace cachebench {

WorkloadGenerator::WorkloadGenerator(const StressorConfig& config)
    : config_{config} {
  for (const auto& c : config.poolDistributions) {
    if (c.keySizeRange.size() != c.keySizeRangeProbability.size() + 1) {
      throw std::invalid_argument(
          "Key size range and their probabilities do not match up. Check your "
          "test config.");
    }
    workloadDist_.push_back(WorkloadDistribution(c));
  }

  if (config_.numKeys > std::numeric_limits<uint32_t>::max()) {
    throw std::invalid_argument(folly::sformat(
        "Too many keys specified: {}. Maximum allowed is 4 Billion.",
        config_.numKeys));
  }

  generateReqs();
  generateKeyDistributions();
}

const Request& WorkloadGenerator::getReq(uint16_t poolId,
                                         std::mt19937_64& gen,
                                         std::optional<uint64_t>) {
  XDCHECK_LT(poolId, keyIndicesForPool_.size());
  XDCHECK_LT(poolId, keyGenForPool_.size());

  size_t idx = keyIndicesForPool_[poolId][keyGenForPool_[poolId](gen)];
  auto op =
      static_cast<OpType>(workloadDist_[workloadIdx(poolId)].sampleOpDist(gen));
  reqs_[idx].setOp(op);
  if (enableRenderDist_)
    reqs_[idx].counter++;
  return reqs_[idx];
}

void WorkloadGenerator::renderDistribution(std::ostream& out) const {
  if (!enableRenderDist_)
    return;

  std::vector<uint64_t> hits;
  uint64_t total_hits = 0;
  for (auto& req: reqs_) {
    if (req.counter)
      hits.push_back(req.counter);
    total_hits += req.counter;
  }
  std::sort(hits.begin(), hits.end(), [](uint64_t a, uint64_t b) {
    return a > b;
  });

  /* fio/t/genzipf.c */
#define DEF_NR_OUTPUT   20
  unsigned long block_size = 4096;
  unsigned long output_nranges = DEF_NR_OUTPUT;
  double percentage = 0;

  struct output_sum {
    double output;
    unsigned int nranges;
  };

  unsigned long nnodes = hits.size();
  unsigned long nranges = total_hits;

  unsigned long i, j, cur_vals, interval_step, next_interval, total_vals;
  unsigned long blocks = percentage * nnodes / 100;
  double hit_percent_sum = 0;
  unsigned long long hit_sum = 0;
  double perc, perc_i;
  struct output_sum *output_sums;

  interval_step = (nnodes - 1) / output_nranges + 1;
  next_interval = interval_step;
  output_sums = new output_sum[output_nranges];

  for (i = 0; i < output_nranges; i++) {
    output_sums[i].output = 0.0;
    output_sums[i].nranges = 0;
  }

  j = total_vals = cur_vals = 0;

  for (i = 0; i < nnodes; i++) {
    struct output_sum *os = &output_sums[j];
    cur_vals += hits[i];
    total_vals += hits[i];
    os->nranges += hits[i];
    if (i == (next_interval) -1 || i == nnodes - 1) {
      os->output = (double) cur_vals / (double) nranges;
      os->output *= 100.0;
      cur_vals = 0;
      next_interval += interval_step;
      j++;
    }

    if (percentage) {
      if (total_vals >= blocks) {
        double cs = (double) i * block_size / (1024.0 * 1024.0);
        char p = 'M';

        if (cs > 1024.0) {
          cs /= 1024.0;
          p = 'G';
        }
        if (cs > 1024.0) {
          cs /= 1024.0;
          p = 'T';
        }

        out << folly::sformat("{:.2f}% of hits satisfied in {:.3f}{}B of cache", percentage, cs, p) << std::endl;
        percentage = 0.0;
      }
    }
  }

  perc_i = 100.0 / (double)output_nranges;
  perc = 0.0;

  out << std::endl << "   Rows           Hits %         Sum %           # Hits          Size" << std::endl;
  out << "-----------------------------------------------------------------------" << std::endl;
  for (i = 0; i < output_nranges; i++) {
    struct output_sum *os = &output_sums[i];
    double gb = (double)os->nranges * block_size / 1024.0;
    char p = 'K';

    if (gb > 1024.0) {
      p = 'M';
      gb /= 1024.0;
    }
    if (gb > 1024.0) {
      p = 'G';
      gb /= 1024.0;
    }

    perc += perc_i;
    hit_percent_sum += os->output;
    hit_sum += os->nranges;
    out << folly::sformat("{} {:6.2f}%\t{:6.2f}%\t\t{:6.2f}%\t\t{:8}\t{:6.2f}{}",
      i ? "|->" : "Top", perc, os->output, hit_percent_sum,
      os->nranges, gb, p) << std::endl;
  }

  out << "-----------------------------------------------------------------------" << std::endl;
  out << folly::sformat("Total\t\t\t\t\t\t{:8}", hit_sum) << std::endl;
  delete output_sums;
}

void WorkloadGenerator::generateKeys() {
  uint16_t pid = 0;
  auto fn = [pid, this](size_t start, size_t end) {
    // All keys are printable lower case english alphabet.
    std::uniform_int_distribution<char> charDis('a', 'z');
    std::mt19937_64 gen(folly::Random::rand64());
    for (uint64_t i = start; i < end; i++) {
      size_t keySize =
          util::narrow_cast<size_t>(workloadDist_[pid].sampleKeySizeDist(gen));
      keys_[i].resize(keySize);
      for (auto& c : keys_[i]) {
        c = charDis(gen);
      }
    }
  };

  size_t totalKeys(0);
  std::chrono::seconds keyGenDuration(0);
  keys_.resize(config_.numKeys);
  for (size_t i = 0; i < config_.keyPoolDistribution.size(); i++) {
    pid = util::narrow_cast<uint16_t>(workloadIdx(i));
    size_t numKeysForPool =
        firstKeyIndexForPool_[i + 1] - firstKeyIndexForPool_[i];
    totalKeys += numKeysForPool;
    keyGenDuration += detail::executeParallel(
        fn, config_.numThreads, numKeysForPool, firstKeyIndexForPool_[i]);
  }

  auto startTime = std::chrono::steady_clock::now();
  for (size_t i = 0; i < config_.keyPoolDistribution.size(); i++) {
    auto poolKeyBegin = keys_.begin() + firstKeyIndexForPool_[i];
    // past the end iterator
    auto poolKeyEnd = keys_.begin() + (firstKeyIndexForPool_[i + 1]);
    std::sort(poolKeyBegin, poolKeyEnd);
    auto newEnd = std::unique(poolKeyBegin, poolKeyEnd);
    // update pool key boundary before invalidating iterators
    for (size_t j = i + 1; j < firstKeyIndexForPool_.size(); j++) {
      firstKeyIndexForPool_[j] -= std::distance(newEnd, poolKeyEnd);
    }
    totalKeys -= std::distance(newEnd, poolKeyEnd);
    keys_.erase(newEnd, poolKeyEnd);
  }
  auto sortDuration = std::chrono::duration_cast<std::chrono::seconds>(
      std::chrono::steady_clock::now() - startTime);

  std::cout << folly::sformat("Created {:,} keys in {:.2f} mins",
                              totalKeys,
                              (keyGenDuration + sortDuration).count() / 60.)
            << std::endl;
}

void WorkloadGenerator::generateReqs() {
  generateFirstKeyIndexForPool();
  generateKeys();
  std::mt19937_64 gen(folly::Random::rand64());
  for (size_t i = 0; i < config_.keyPoolDistribution.size(); i++) {
    size_t idx = workloadIdx(i);
    for (size_t j = firstKeyIndexForPool_[i]; j < firstKeyIndexForPool_[i + 1];
         j++) {
      std::vector<size_t> chainSizes;
      chainSizes.push_back(
          util::narrow_cast<size_t>(workloadDist_[idx].sampleValDist(gen)));
      int chainLen =
          util::narrow_cast<int>(workloadDist_[idx].sampleChainedLenDist(gen));
      for (int k = 0; k < chainLen; k++) {
        chainSizes.push_back(util::narrow_cast<size_t>(
            workloadDist_[idx].sampleChainedValDist(gen)));
      }
      sizes_.emplace_back(chainSizes);
      auto reqSizes = sizes_.end() - 1;
      reqs_.emplace_back(keys_[j], reqSizes->begin(), reqSizes->end());
    }
  }
}

void WorkloadGenerator::generateFirstKeyIndexForPool() {
  auto sumProb = std::accumulate(config_.keyPoolDistribution.begin(),
                                 config_.keyPoolDistribution.end(), 0.);
  auto accumProb = 0.;
  firstKeyIndexForPool_.push_back(0);
  for (auto prob : config_.keyPoolDistribution) {
    accumProb += prob;
    firstKeyIndexForPool_.push_back(
        util::narrow_cast<uint32_t>(config_.numKeys * accumProb / sumProb));
  }
}

void WorkloadGenerator::generateKeyDistributions() {
  // We are trying to generate a gaussian distribution for each pool's part
  // in the overall cache ops. To keep the amount of memory finite, we only
  // generate a max of 4 billion op traces across all the pools and replay
  // the same when we need longer traces.
  std::chrono::seconds duration{0};
  for (uint64_t i = 0; i < config_.opPoolDistribution.size(); i++) {
    auto left = firstKeyIndexForPool_[i];
    auto right = firstKeyIndexForPool_[i + 1] - 1;
    size_t idx = workloadIdx(i);

    size_t numOpsForPool = std::min<size_t>(
        util::narrow_cast<size_t>(config_.numOps * config_.numThreads *
                                  config_.opPoolDistribution[i]),
        std::numeric_limits<uint32_t>::max());
    std::cout << folly::sformat("Generating {:.2f}M sampled accesses",
                                numOpsForPool / 1e6)
              << std::endl;
    keyGenForPool_.push_back(std::uniform_int_distribution<uint32_t>(
        0, util::narrow_cast<uint32_t>(numOpsForPool) - 1));
    keyIndicesForPool_.push_back(std::vector<uint32_t>(numOpsForPool));

    duration += detail::executeParallel(
        [&, this](size_t start, size_t end) {
          std::mt19937_64 gen(folly::Random::rand64());
          auto popDist = workloadDist_[idx].getPopDist(left, right);
          for (uint64_t j = start; j < end; j++) {
            keyIndicesForPool_[i][j] =
                util::narrow_cast<uint32_t>((*popDist)(gen));
          }
        },
        config_.numThreads, numOpsForPool);
  }

  std::cout << folly::sformat("Generated access patterns in {:.2f} mins",
                              duration.count() / 60.)
            << std::endl;
}

} // namespace cachebench
} // namespace cachelib
} // namespace facebook
