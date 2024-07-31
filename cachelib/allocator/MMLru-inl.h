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

namespace facebook {
namespace cachelib {
namespace detail {
template <typename T>
bool areBytesSame(const T& one, const T& two) {
  return std::memcmp(&one, &two, sizeof(T)) == 0;
}
} // namespace detail

/* Container Interface Implementation */
template <typename T, MMLru::Hook<T> T::*HookPtr>
MMLru::Container<T, HookPtr>::Container(serialization::MMLruObject object,
                                        PtrCompressor compressor, Prefetcher prefetcher)
    : compressor_(std::move(compressor)),
      prefetcher_(std::move(prefetcher)),
      config_(*object.config()) {
  for (int shard = 0; shard < kShards; shard++) {
    lru_.emplace_back(std::make_unique<Lru>(*object.lru(), compressor_, prefetcher_));
    auto lru = lru_[shard].get();
    lru->tailSize_ = *object.tailSize();
    lru->insertionPoint_ = compressor_.unCompress(
          CompressedPtr{*object.compressedInsertionPoint()});
    lru->lruRefreshTime_ = config_.lruRefreshTime;
    lru->nextReconfigureTime_ = config_.mmReconfigureIntervalSecs.count() == 0
                             ? std::numeric_limits<Time>::max()
                             : static_cast<Time>(util::getCurrentTimeSec()) +
                                   config_.mmReconfigureIntervalSecs.count();
  }
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
bool MMLru::Container<T, HookPtr>::recordAccess(T& node,
                                                AccessMode mode) noexcept {
  if ((mode == AccessMode::kWrite && !config_.updateOnWrite) ||
      (mode == AccessMode::kRead && !config_.updateOnRead)) {
    return false;
  }

  prefetcher_.accessMMContainerRecordAccess(&node);
  const auto curr = static_cast<Time>(util::getCurrentTimeSec());
  // check if the node is still being memory managed
  if (!node.isInMMContainer())
    return false;

  auto shard = getShard(node);
  auto lru = lru_[shard].get();
  if (((curr >= getUpdateTime(node) +
                    lru->lruRefreshTime_.load(std::memory_order_relaxed)) ||
       !isAccessed(node))) {
    if (!isAccessed(node)) {
      markAccessed(node);
    }

    auto func = [this, &node, curr, shard]() {
      auto lru = lru_[shard].get();
      reconfigureLocked(shard, curr);
      ensureNotInsertionPoint(node);
      if (node.isInMMContainer()) {
        lru->list_.moveToHead(node);
        setUpdateTime(node, curr);
      }
      if (isTail(node)) {
        unmarkTail(node);
        lru->tailSize_--;
        XDCHECK_LE(0u, lru->tailSize_);
        updateLruInsertionPoint(shard);
      }
    };

    // if the tryLockUpdate optimization is on, and we were able to grab the
    // lock, execute the critical section and return true, else return false
    //
    // if the tryLockUpdate optimization is off, we always execute the
    // critical section and return true
    if (config_.tryLockUpdate) {
      if (auto lck = LockHolder{*lru->lruMutex_, std::try_to_lock}) {
        func();
        return true;
      }

      return false;
    }

    lru->lruMutex_->lock_combine(func);
    return true;
  }
  return false;
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
cachelib::EvictionAgeStat MMLru::Container<T, HookPtr>::getEvictionAgeStat(
    uint64_t projectedLength) const noexcept {
  EvictionAgeStat totalStat{};
  for (int shard = 0; shard < kShards; shard++) {
    auto stat = lru_[shard].get()->lruMutex_->lock_combine([this, shard, projectedLength]() {
      return getEvictionAgeStatLocked(shard, projectedLength);
    });
    totalStat.warmQueueStat += stat.warmQueueStat;
  }
  return totalStat;
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
cachelib::EvictionAgeStat
MMLru::Container<T, HookPtr>::getEvictionAgeStatLocked(
    int shard,
    uint64_t projectedLength) const noexcept {
  EvictionAgeStat stat{};
  const auto currTime = static_cast<Time>(util::getCurrentTimeSec());

  auto lru = lru_[shard].get();
  const T* node = lru->list_.getTail();
  stat.warmQueueStat.oldestElementAge =
      node ? currTime - getUpdateTime(*node) : 0;
  for (size_t numSeen = 0; numSeen < projectedLength && node != nullptr;
       numSeen++, node = lru->list_.getPrev(*node)) {
  }
  stat.warmQueueStat.projectedAge = node ? currTime - getUpdateTime(*node)
                                         : stat.warmQueueStat.oldestElementAge;
  XDCHECK(detail::areBytesSame(stat.hotQueueStat, EvictionStatPerType{}));
  XDCHECK(detail::areBytesSame(stat.coldQueueStat, EvictionStatPerType{}));
  return stat;
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
void MMLru::Container<T, HookPtr>::setConfig(const Config& newConfig) {
  config_ = newConfig;
  for (int shard = 0; shard < kShards; shard++) {
    lru_[shard].get()->lruMutex_->lock_combine([this, shard, newConfig]() {
      setConfigLocked(shard, newConfig);
    });
  }
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
void MMLru::Container<T, HookPtr>::setConfigLocked(int shard, const Config& newConfig) {
    auto lru = lru_[shard].get();
    if (config_.lruInsertionPointSpec == 0 && lru->insertionPoint_ != nullptr) {
      auto curr = lru->insertionPoint_;
      while (lru->tailSize_ != 0) {
        XDCHECK(curr != nullptr);
        unmarkTail(*curr);
        lru->tailSize_--;
        curr = lru->list_.getNext(*curr);
      }
      lru->insertionPoint_ = nullptr;
    }
    lru->lruRefreshTime_.store(config_.lruRefreshTime, std::memory_order_relaxed);
    lru->nextReconfigureTime_ = config_.mmReconfigureIntervalSecs.count() == 0
                               ? std::numeric_limits<Time>::max()
                               : static_cast<Time>(util::getCurrentTimeSec()) +
                                     config_.mmReconfigureIntervalSecs.count();
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
typename MMLru::Config MMLru::Container<T, HookPtr>::getConfig() const {
  // todo: introduce config lock
  return lru_[0].get()->lruMutex_->lock_combine([this]() { return config_; });
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
void MMLru::Container<T, HookPtr>::updateLruInsertionPoint(int shard) noexcept {
  if (config_.lruInsertionPointSpec == 0) {
    return;
  }

  auto lru = lru_[shard].get();
  // If insertionPoint_ is nullptr initialize it to tail first
  if (lru->insertionPoint_ == nullptr) {
    lru->insertionPoint_ = lru->list_.getTail();
    lru->tailSize_ = 0;
    if (lru->insertionPoint_ != nullptr) {
      prefetcher_.accessMMContainerUpdateInsertionPoint(lru->insertionPoint_);
      markTail(*lru->insertionPoint_);
      lru->tailSize_++;
    }
  }

  if (lru->list_.size() <= 1) {
    // we are done;
    return;
  }

  XDCHECK_NE(reinterpret_cast<uintptr_t>(nullptr),
             reinterpret_cast<uintptr_t>(lru->insertionPoint_));

  const auto expectedSize = lru->list_.size() >> config_.lruInsertionPointSpec;
  auto curr = lru->insertionPoint_;

  while (lru->tailSize_ < expectedSize && curr != lru->list_.getHead()) {
    curr = lru->list_.getPrev(*curr);
    markTail(*curr);
    lru->tailSize_++;
  }

  while (lru->tailSize_ > expectedSize && curr != lru->list_.getTail()) {
    unmarkTail(*curr);
    lru->tailSize_--;
    curr = lru->list_.getNext(*curr);
  }

  lru->insertionPoint_ = curr;
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
bool MMLru::Container<T, HookPtr>::add(T& node, uint64_t hash) noexcept {
  const auto currTime = static_cast<Time>(util::getCurrentTimeSec());
  const int shard = hash % kShards;

  return lru_[shard].get()->lruMutex_->lock_combine([this, &node, currTime, shard]() {
    if (node.isInMMContainer()) {
      return false;
    }
    setShard(node, shard);
    auto lru = lru_[shard].get();
    if (config_.lruInsertionPointSpec == 0 || lru->insertionPoint_ == nullptr) {
      lru->list_.linkAtHead(node);
    } else {
      lru->list_.insertBefore(*lru->insertionPoint_, node);
    }
    node.markInMMContainer();
    setUpdateTime(node, currTime);
    unmarkAccessed(node);
    updateLruInsertionPoint(shard);
    return true;
  });
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
typename MMLru::Container<T, HookPtr>::LockedIterator
MMLru::Container<T, HookPtr>::getEvictionIterator() const noexcept {
  auto lru = lru_[folly::Random::rand32(0, kShards)].get();
  LockHolder l(*lru->lruMutex_);
  return LockedIterator{std::move(l), lru->list_.rbegin()};
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
template <typename F>
void MMLru::Container<T, HookPtr>::withEvictionIterator(F&& fun) {
  auto lru = lru_[folly::Random::rand32(0, kShards)].get();
  if (config_.useCombinedLockForIterators) {
    lru->lruMutex_->lock_combine([this, &fun, lru]() { fun(Iterator{lru->list_.rbegin()}); });
  } else {
    LockHolder lck{*lru->lruMutex_};
    fun(Iterator{lru->list_.rbegin()});
  }
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
void MMLru::Container<T, HookPtr>::ensureNotInsertionPoint(T& node) noexcept {
  // If we are removing the insertion point node, grow tail before we remove
  // so that insertionPoint_ is valid (or nullptr) after removal
  auto lru = lru_[getShard(node)].get();
  if (&node == lru->insertionPoint_) {
    lru->insertionPoint_ = lru->list_.getPrev(*lru->insertionPoint_);
    if (lru->insertionPoint_ != nullptr) {
      lru->tailSize_++;
      markTail(*lru->insertionPoint_);
    } else {
      XDCHECK_EQ(lru->list_.size(), 1u);
    }
  }
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
void MMLru::Container<T, HookPtr>::removeLocked(T& node) {
  int shard = getShard(node);
  auto lru = lru_[shard].get();
  ensureNotInsertionPoint(node);
  lru->list_.remove(node);
  unmarkAccessed(node);
  if (isTail(node)) {
    unmarkTail(node);
    lru->tailSize_--;
  }
  node.unmarkInMMContainer();
  updateLruInsertionPoint(shard);
  return;
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
bool MMLru::Container<T, HookPtr>::remove(T& node) noexcept {
  auto lru = lru_[getShard(node)].get();
  return lru->lruMutex_->lock_combine([this, &node]() {
    if (!node.isInMMContainer()) {
      return false;
    }
    removeLocked(node);
    return true;
  });
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
void MMLru::Container<T, HookPtr>::remove(Iterator& it) noexcept {
  T& node = *it;
  XDCHECK(node.isInMMContainer());
  ++it;
  removeLocked(node);
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
bool MMLru::Container<T, HookPtr>::replace(T& oldNode, T& newNode) noexcept {
  const auto shard = getShard(oldNode);
  auto lru = lru_[shard].get();
  return lru->lruMutex_->lock_combine([this, &oldNode, &newNode, shard]() {
    if (!oldNode.isInMMContainer() || newNode.isInMMContainer()) {
      return false;
    }
    auto lru = lru_[shard].get();
    setShard(newNode, shard);
    const auto updateTime = getUpdateTime(oldNode);
    lru->list_.replace(oldNode, newNode);
    oldNode.unmarkInMMContainer();
    newNode.markInMMContainer();
    setUpdateTime(newNode, updateTime);
    if (isAccessed(oldNode)) {
      markAccessed(newNode);
    } else {
      unmarkAccessed(newNode);
    }
    XDCHECK(!isTail(newNode));
    if (isTail(oldNode)) {
      markTail(newNode);
      unmarkTail(oldNode);
    } else {
      unmarkTail(newNode);
    }
    if (lru->insertionPoint_ == &oldNode) {
      lru->insertionPoint_ = &newNode;
    }
    return true;
  });
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
serialization::MMLruObject MMLru::Container<T, HookPtr>::saveState()
    const noexcept {
  serialization::MMLruConfig configObject;
  *configObject.lruRefreshTime() =
      lru_[0].get()->lruRefreshTime_.load(std::memory_order_relaxed);
  *configObject.lruRefreshRatio() = config_.lruRefreshRatio;
  *configObject.updateOnWrite() = config_.updateOnWrite;
  *configObject.updateOnRead() = config_.updateOnRead;
  *configObject.tryLockUpdate() = config_.tryLockUpdate;
  *configObject.lruInsertionPointSpec() = config_.lruInsertionPointSpec;

  serialization::MMLruObject object;
  *object.config() = configObject;

  // todo: update serialization::MMLruObject
  for (int shard = 0; shard < kShards; shard++) {
    auto lru = lru_[shard].get();
    *object.compressedInsertionPoint() =
      compressor_.compress(lru->insertionPoint_).saveState();
    *object.tailSize() = lru->tailSize_;
    *object.lru() = lru->list_.saveState();
  }
  return object;
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
MMContainerStat MMLru::Container<T, HookPtr>::getStats() const noexcept {
  MMContainerStat stats{};
  for (int shard = 0; shard < kShards; shard++) {
    auto lru = lru_[shard].get();
    auto stat = lru->lruMutex_->lock_combine([this, lru]() {
      auto* tail = lru->list_.getTail();

    // we return by array here because DistributedMutex is fastest when the
    // output data fits within 48 bytes.  And the array is exactly 48 bytes, so
    // it can get optimized by the implementation.
    //
    // the rest of the parameters are 0, so we don't need the critical section
    // to return them
      return folly::make_array(lru->list_.size(),
                             tail == nullptr ? 0 : getUpdateTime(*tail),
                             lru->lruRefreshTime_.load(std::memory_order_relaxed));
    });
    stats.size += stat[0];
    stats.oldestTimeSec = std::max(stats.oldestTimeSec, stat[1]);
    stats.lruRefreshTime = !stats.lruRefreshTime ? stat[2] : std::min(stats.lruRefreshTime, stat[2]);
  }
  return stats;
}

template <typename T, MMLru::Hook<T> T::*HookPtr>
void MMLru::Container<T, HookPtr>::reconfigureLocked(int shard, const Time& currTime) {
  auto lru = lru_[shard].get();
  if (currTime < lru->nextReconfigureTime_) {
    return;
  }
  lru->nextReconfigureTime_ = currTime + config_.mmReconfigureIntervalSecs.count();

  // update LRU refresh time
  auto stat = getEvictionAgeStatLocked(shard, 0);
  auto lruRefreshTime = std::min(
      std::max(config_.defaultLruRefreshTime,
               static_cast<uint32_t>(stat.warmQueueStat.oldestElementAge *
                                     config_.lruRefreshRatio)),
      kLruRefreshTimeCap);
  lru->lruRefreshTime_.store(lruRefreshTime, std::memory_order_relaxed);
}

// Iterator Context Implementation
template <typename T, MMLru::Hook<T> T::*HookPtr>
MMLru::Container<T, HookPtr>::LockedIterator::LockedIterator(
    LockHolder l, const Iterator& iter) noexcept
    : Iterator(iter), l_(std::move(l)) {}

} // namespace cachelib
} // namespace facebook
