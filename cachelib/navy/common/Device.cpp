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

#include "cachelib/allocator/memory/Prefetcher.h"
#include "cachelib/navy/common/Device.h"

#include <folly/File.h>
#include <folly/Format.h>
#include <folly/ThreadLocal.h>
#include <folly/experimental/io/IoUring.h>
#include <folly/fibers/FiberManager.h>

#include <cstring>
#include <numeric>
#include <iostream>

namespace facebook {
namespace cachelib {
namespace navy {
namespace {

using IOOperation =
    std::function<ssize_t(int fdIdx, void* buf, size_t count, off_t offset)>;

// Device on Unix file descriptor
class FileDevice final : public Device {
 public:
  FileDevice(folly::File file,
             uint64_t size,
             uint32_t ioAlignSize,
             std::shared_ptr<DeviceEncryptor> encryptor,
             uint32_t maxDeviceWriteSize)
      : Device{size, std::move(encryptor), ioAlignSize, maxDeviceWriteSize},
        file_{std::move(file)} {}
  FileDevice(const FileDevice&) = delete;
  FileDevice& operator=(const FileDevice&) = delete;

  ~FileDevice() override {}

 private:
  folly::IoUring* getIoUring() {
    folly::IoUring* ioUring = ioUring_.get();
    if (!ioUring) {
      ioUring = new folly::IoUring(1024, folly::AsyncBase::NOT_POLLABLE, 128);
      ioUring->initializeContext();
      ioUring_.reset(ioUring);
    }
    return ioUring;
  }

  bool writeImpl(uint64_t offset, uint32_t size, const void* value) override {
    if (folly::fibers::onFiber()) {
      folly::IoUring* ioUring = getIoUring();
      folly::IoUring::Op writeOp;
      size_t completed = 0;

      writeOp.setNotificationCallback(
          [&](folly::AsyncBaseOp*) { ++completed; });
      writeOp.pwrite(file_.fd(), value, size, offset);
      CheckpointTracer::record(kWriteSubmitBegin);
      ioUring->submit(&writeOp);
      CheckpointTracer::record(kWriteSubmitEnd);
      do {
        if (ioUring->wait(0).size() && completed) {
          break;
        }
        CheckpointTracer::record(kWriteYieldBegin);
        folly::fibers::yield();
        CheckpointTracer::record(kWriteYieldEnd);
      } while (!completed);
      ssize_t bytesWritten = writeOp.result();
      if (bytesWritten != size) {
        reportIOError("write", offset, size, bytesWritten);
      }
      return bytesWritten == size;
    }

    CheckpointTracer::record(kWriteBegin);
    ssize_t bytesWritten = ::pwrite(file_.fd(), value, size, offset);
    CheckpointTracer::record(kWriteEnd);
    if (bytesWritten != size) {
      reportIOError("write", offset, size, bytesWritten);
    }
    return bytesWritten == size;
  }

  bool readImpl(uint64_t offset, uint32_t size, void* value) override {
    if (folly::fibers::onFiber()) {
      folly::IoUring* ioUring = getIoUring();
      folly::IoUring::Op readOp;
      size_t completed = 0;

      readOp.setNotificationCallback(
          [&](folly::AsyncBaseOp*) { ++completed; });
      readOp.pread(file_.fd(), value, size, offset);
      CheckpointTracer::record(kReadSubmitBegin);
      ioUring->submit(&readOp);
      CheckpointTracer::record(kReadSubmitEnd);
      do {
        if (ioUring->wait(0).size() && completed) {
          break;
        }
        CheckpointTracer::record(kReadYieldBegin);
        folly::fibers::yield();
        CheckpointTracer::record(kReadYieldEnd);
      } while (!completed);
      ssize_t bytesRead = readOp.result();
      if (bytesRead != size) {
        reportIOError("read", offset, size, bytesRead);
      }
      return bytesRead == size;
    }

    CheckpointTracer::record(kReadBegin);
    ssize_t bytesRead = ::pread(file_.fd(), value, size, offset);
    CheckpointTracer::record(kReadEnd);
    if (bytesRead != size) {
      reportIOError("read", offset, size, bytesRead);
    }
    return bytesRead == size;
  }

  void flushImpl() override { ::fsync(file_.fd()); }

  void reportIOError(const char* opName,
                     uint64_t offset,
                     uint32_t size,
                     ssize_t ioRet) {
    XLOG_EVERY_N_THREAD(
        ERR, 1000,
        folly::sformat("IO error: {} offset={} size={} ret={} errno={} ({})",
                       opName, offset, size, ioRet, errno,
                       std::strerror(errno)));
  }

  const folly::File file_{};
  folly::ThreadLocalPtr<folly::IoUring> ioUring_;
};

// RAID0 device spanning multiple files
class RAID0Device final : public Device {
 public:
  RAID0Device(std::vector<folly::File> fvec,
              uint64_t fdSize,
              uint32_t ioAlignSize,
              uint32_t stripeSize,
              std::shared_ptr<DeviceEncryptor> encryptor,
              uint32_t maxDeviceWriteSize)
      : Device{fdSize * fvec.size(), std::move(encryptor), ioAlignSize,
               maxDeviceWriteSize},
        fvec_{std::move(fvec)},
        ioUringVec_(fvec_.size()),
        stripeSize_(stripeSize) {
    XDCHECK_GT(ioAlignSize, 0u);
    XDCHECK_GT(stripeSize_, 0u);
    XDCHECK_GE(stripeSize_, ioAlignSize);
    XDCHECK_EQ(0u, stripeSize_ % 2) << stripeSize_;
    XDCHECK_EQ(0u, stripeSize_ % ioAlignSize)
        << stripeSize_ << ", " << ioAlignSize;
    if (fdSize % stripeSize != 0) {
      throw std::invalid_argument(
          folly::sformat("Invalid size because individual device size: {} is "
                         "not aligned to stripe size: {}",
                         fdSize, stripeSize));
    }
  }
  RAID0Device(const RAID0Device&) = delete;
  RAID0Device& operator=(const RAID0Device&) = delete;

  ~RAID0Device() override {}

 private:
  folly::IoUring* getIoUring(int fdIdx) {
    folly::IoUring* ioUring = ioUringVec_[fdIdx].get();
    if (!ioUring) {
      ioUring = new folly::IoUring(1024, folly::AsyncBase::NOT_POLLABLE, 128);
      ioUring->initializeContext();
      ioUringVec_[fdIdx].reset(ioUring);
    }
    return ioUring;
  }

  bool writeImpl(uint64_t offset, uint32_t size, const void* value) override {
    IOOperation io;
    if (folly::fibers::onFiber()) {
      io = [this](int fdIdx, void* buf, size_t count, off_t offset) {
          folly::IoUring* ioUring = getIoUring(fdIdx);
          folly::IoUring::Op writeOp;
          size_t completed = 0;

          writeOp.setNotificationCallback(
              [&](folly::AsyncBaseOp*) { ++completed; });
          writeOp.pwrite(fvec_[fdIdx].fd(), buf, count, offset);
          CheckpointTracer::record(kWriteSubmitBegin);
          ioUring->submit(&writeOp);
          CheckpointTracer::record(kWriteSubmitEnd);
          do {
            CheckpointTracer::record(kWriteYieldBegin);
            folly::fibers::yield();
            CheckpointTracer::record(kWriteYieldEnd);
            if (completed) {
              break;
            }
            ioUring->wait(0);
          } while (!completed);
          return writeOp.result();
      };
    } else {
      io = [this](int fdIdx, void* buf, size_t count, off_t offset) {
        CheckpointTracer::record(kWriteBegin);
        auto ret = ::pwrite(fvec_[fdIdx].fd(), buf, count, offset);
        CheckpointTracer::record(kWriteEnd);
        return ret;
      };
    }
    return doIO(offset, size, const_cast<void*>(value), "RAID0 WRITE", io);
  }

  bool readImpl(uint64_t offset, uint32_t size, void* value) override {
    IOOperation io;
    if (folly::fibers::onFiber()) {
      io = [this](int fdIdx, void* buf, size_t count, off_t offset) {
          folly::IoUring* ioUring = getIoUring(fdIdx);
          folly::IoUring::Op readOp;
          size_t completed = 0;

          readOp.setNotificationCallback(
              [&](folly::AsyncBaseOp*) { ++completed; });
          readOp.pread(fvec_[fdIdx].fd(), buf, count, offset);
          CheckpointTracer::record(kReadSubmitBegin);
          ioUring->submit(&readOp);
          CheckpointTracer::record(kReadSubmitEnd);
          do {
            CheckpointTracer::record(kReadYieldBegin);
            folly::fibers::yield();
            CheckpointTracer::record(kReadYieldEnd);
            if (completed) {
              break;
            }
            ioUring->wait(0);
          } while (!completed);
          return readOp.result();
      };
    } else {
      io = [this](int fdIdx, void* buf, size_t count, off_t offset) {
          CheckpointTracer::record(kReadBegin);
          auto ret = ::pread(fvec_[fdIdx].fd(), buf, count, offset);
          CheckpointTracer::record(kReadEnd);
          return ret;
      };
    }
    return doIO(offset, size, value, "RAID0 READ", io);
  }

  void flushImpl() override {
    for (const auto& f : fvec_) {
      ::fsync(f.fd());
    }
  }

  bool doIO(uint64_t offset,
            uint32_t size,
            void* value,
            const char* opName,
            IOOperation& io) {
    uint8_t* buf = reinterpret_cast<uint8_t*>(value);

    while (size > 0) {
      uint64_t stripe = offset / stripeSize_;
      uint32_t fdIdx = stripe % fvec_.size();
      uint64_t stripeStartOffset = (stripe / fvec_.size()) * stripeSize_;
      uint32_t ioOffsetInStripe = offset % stripeSize_;
      uint32_t allowedIOSize = std::min(size, stripeSize_ - ioOffsetInStripe);

      ssize_t retSize = io(fdIdx,
                           buf,
                           allowedIOSize,
                           stripeStartOffset + ioOffsetInStripe);
      if (retSize != allowedIOSize) {
        XLOG_EVERY_N_THREAD(
            ERR, 1000,
            folly::sformat(
                "IO error: {} logicalOffset={} logicalIOSize={} stripeSize={} "
                "stripe={} offsetInStripe={} stripeIOSize={} ret={} errno={} "
                "({})",
                opName,
                offset,
                size,
                stripeSize_,
                stripe,
                ioOffsetInStripe,
                allowedIOSize,
                retSize,
                errno,
                std::strerror(errno)));

        return false;
      }

      size -= allowedIOSize;
      offset += allowedIOSize;
      buf += allowedIOSize;
    }
    return true;
  }

  const std::vector<folly::File> fvec_{};
  std::vector<folly::ThreadLocalPtr<folly::IoUring>> ioUringVec_;
  const uint32_t stripeSize_{};
};

// Device on memory buffer
class MemoryDevice final : public Device {
 public:
  explicit MemoryDevice(uint64_t size,
                        std::shared_ptr<DeviceEncryptor> encryptor,
                        uint32_t ioAlignSize)
      : Device{size, std::move(encryptor), ioAlignSize,
               0 /* max device write size */},
        buffer_{std::make_unique<uint8_t[]>(size)} {}
  MemoryDevice(const MemoryDevice&) = delete;
  MemoryDevice& operator=(const MemoryDevice&) = delete;
  ~MemoryDevice() override = default;

 private:
  bool writeImpl(uint64_t offset,
                 uint32_t size,
                 const void* value) noexcept override {
    XDCHECK_LE(offset + size, getSize());
    std::memcpy(buffer_.get() + offset, value, size);
    return true;
  }

  bool readImpl(uint64_t offset, uint32_t size, void* value) override {
    XDCHECK_LE(offset + size, getSize());
    std::memcpy(value, buffer_.get() + offset, size);
    return true;
  }

  void flushImpl() override {
    // Noop
  }

  std::unique_ptr<uint8_t[]> buffer_;
};
} // namespace

bool Device::write(uint64_t offset, BufferView view) {
  if (encryptor_) {
    auto writeBuffer = makeIOBuffer(view.size());
    writeBuffer.copyFrom(0, view);
    return write(offset, std::move(writeBuffer));
  }

  const auto size = view.size();
  XDCHECK_LE(offset + size, size_);
  const uint8_t* data = reinterpret_cast<const uint8_t*>(view.data());
  return writeInternal(offset, data, size);
}

bool Device::write(uint64_t offset, Buffer buffer) {
  const auto size = buffer.size();
  XDCHECK_LE(offset + buffer.size(), size_);
  uint8_t* data = reinterpret_cast<uint8_t*>(buffer.data());
  XDCHECK_EQ(reinterpret_cast<uint64_t>(data) % ioAlignmentSize_, 0ul);
  if (encryptor_) {
    XCHECK_EQ(offset % encryptor_->encryptionBlockSize(), 0ul);
    auto res = encryptor_->encrypt(folly::MutableByteRange{data, size}, offset);
    if (!res) {
      encryptionErrors_.inc();
      return false;
    }
  }
  return writeInternal(offset, data, size);
}

bool Device::writeInternal(uint64_t offset, const uint8_t* data, size_t size) {
  auto remainingSize = size;
  auto maxWriteSize = (maxWriteSize_ == 0) ? remainingSize : maxWriteSize_;
  bool result = true;
  while (remainingSize > 0) {
    auto writeSize = std::min<size_t>(maxWriteSize, remainingSize);
    XDCHECK_EQ(offset % ioAlignmentSize_, 0ul);
    XDCHECK_EQ(writeSize % ioAlignmentSize_, 0ul);

    auto timeBegin = getSteadyClock();
    result = writeImpl(offset, writeSize, data);
    writeLatencyEstimator_.trackValue(
        toMicros((getSteadyClock() - timeBegin)).count());

    if (result) {
      bytesWritten_.add(writeSize);
    } else {
      // One part of the write failed so we abort the rest
      break;
    }
    offset += writeSize;
    data += writeSize;
    remainingSize -= writeSize;
  }
  if (!result) {
    writeIOErrors_.inc();
  }
  return result;
}

// reads size number of bytes from the device from the offset into value.
// Both offset and size are expected to be aligned for device IO operations.
// If successful and encryptor_ is defined, size bytes from
// validDataOffsetInValue offset in value are decrypted.
//
// returns true if successful, false otherwise.
bool Device::readInternal(uint64_t offset, uint32_t size, void* value) {
  XDCHECK_EQ(reinterpret_cast<uint64_t>(value) % ioAlignmentSize_, 0ul);
  XDCHECK_EQ(offset % ioAlignmentSize_, 0ul);
  XDCHECK_EQ(size % ioAlignmentSize_, 0ul);
  XDCHECK_LE(offset + size, size_);
  auto timeBegin = getSteadyClock();
  bool result = readImpl(offset, size, value);
  readLatencyEstimator_.trackValue(
      toMicros(getSteadyClock() - timeBegin).count());
  if (!result) {
    readIOErrors_.inc();
    return result;
  }
  bytesRead_.add(size);
  if (encryptor_) {
    XCHECK_EQ(offset % encryptor_->encryptionBlockSize(), 0ul);
    auto res = encryptor_->decrypt(
        folly::MutableByteRange{reinterpret_cast<uint8_t*>(value), size},
        offset);
    if (!res) {
      decryptionErrors_.inc();
      return false;
    }
  }
  return true;
}

// This API reads size bytes from the Device from offset into a Buffer and
// returns the Buffer. If offset and size are not aligned to device's
// ioAlignmentSize_, IO aligned offset and IO aligned size are determined
// and passed to device read. Upon successful read from the device, the
// buffer is adjusted to return the intended data by trimming the data in
// the front and back.
// An empty buffer is returned in case of error and the caller must check
// the buffer size returned with size passed in to check for errors.
Buffer Device::read(uint64_t offset, uint32_t size) {
  XDCHECK_LE(offset + size, size_);
  uint64_t readOffset =
      offset & ~(static_cast<uint64_t>(ioAlignmentSize_) - 1ul);
  uint64_t readPrefixSize =
      offset & (static_cast<uint64_t>(ioAlignmentSize_) - 1ul);
  auto readSize = getIOAlignedSize(readPrefixSize + size);
  auto buffer = makeIOBuffer(readSize);
  bool result = readInternal(readOffset, readSize, buffer.data());
  if (!result) {
    return Buffer{};
  }
  buffer.trimStart(readPrefixSize);
  buffer.shrink(size);
  return buffer;
}

// This API reads size bytes from the Device from the offset into value.
// Both offset and size are expected to be IO aligned.
bool Device::read(uint64_t offset, uint32_t size, void* value) {
  return readInternal(offset, size, value);
}

void Device::getCounters(const CounterVisitor& visitor) const {
  visitor("navy_device_bytes_written", getBytesWritten(),
          CounterVisitor::CounterType::RATE);
  visitor("navy_device_bytes_read", getBytesRead(),
          CounterVisitor::CounterType::RATE);
  readLatencyEstimator_.visitQuantileEstimator(visitor,
                                               "navy_device_read_latency_us");
  writeLatencyEstimator_.visitQuantileEstimator(visitor,
                                                "navy_device_write_latency_us");
  visitor("navy_device_read_errors", readIOErrors_.get(),
          CounterVisitor::CounterType::RATE);
  visitor("navy_device_write_errors", writeIOErrors_.get(),
          CounterVisitor::CounterType::RATE);
  visitor("navy_device_encryption_errors", encryptionErrors_.get(),
          CounterVisitor::CounterType::RATE);
  visitor("navy_device_decryption_errors", decryptionErrors_.get(),
          CounterVisitor::CounterType::RATE);
}

std::unique_ptr<Device> createFileDevice(
    folly::File file,
    uint64_t size,
    std::shared_ptr<DeviceEncryptor> encryptor) {
  return std::make_unique<FileDevice>(std::move(file), size, 0,
                                      std::move(encryptor),
                                      0 /* max device write size */);
}

std::unique_ptr<Device> createDirectIoFileDevice(
    folly::File file,
    uint64_t size,
    uint32_t ioAlignSize,
    std::shared_ptr<DeviceEncryptor> encryptor,
    uint32_t maxDeviceWriteSize) {
  XDCHECK(folly::isPowTwo(ioAlignSize));
  return std::make_unique<FileDevice>(std::move(file), size, ioAlignSize,
                                      std::move(encryptor), maxDeviceWriteSize);
}

std::unique_ptr<Device> createDirectIoRAID0Device(
    std::vector<folly::File> fvec,
    uint64_t size, // size of each device in the RAID
    uint32_t ioAlignSize,
    uint32_t stripeSize,
    std::shared_ptr<DeviceEncryptor> encryptor,
    uint32_t maxDeviceWriteSize) {
  XDCHECK(folly::isPowTwo(ioAlignSize));
  return std::make_unique<RAID0Device>(std::move(fvec), size, ioAlignSize,
                                       stripeSize, std::move(encryptor),
                                       maxDeviceWriteSize);
}

std::unique_ptr<Device> createMemoryDevice(
    uint64_t size,
    std::shared_ptr<DeviceEncryptor> encryptor,
    uint32_t ioAlignSize) {
  return std::make_unique<MemoryDevice>(size, std::move(encryptor),
                                        ioAlignSize);
}
} // namespace navy
} // namespace cachelib
} // namespace facebook
