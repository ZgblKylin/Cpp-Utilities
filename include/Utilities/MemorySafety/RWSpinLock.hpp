#ifndef CPP_UTILITIES_MEMORYSAFETY_RWSPINLOCK_HPP
#define CPP_UTILITIES_MEMORYSAFETY_RWSPINLOCK_HPP
/*
 * Copyright 2011-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * \file RWSpinLock.hpp
 * \brief ReadWrite lock using spin_lock, copied from Folly library under
 *        Apache-2.0 license, see https://github.com/facebook/folly/blob/master/folly/synchronization/RWSpinLock.h
 * \details
 * \author Xin Liu <xliux@fb.com>
 *
 * N.B. You most likely do _not_ want to use RWSpinLock or any other
 * kind of spinlock.  Use SharedMutex instead.
 *
 * In short, spinlocks in preemptive multi-tasking operating systems
 * have serious problems and fast mutexes like SharedMutex are almost
 * certainly the better choice, because letting the OS scheduler put a
 * thread to sleep is better for system responsiveness and throughput
 * than wasting a timeslice repeatedly querying a lock held by a
 * thread that's blocked, and you can't prevent userspace
 * programs blocking.
 *
 * Spinlocks in an operating system kernel make much more sense than
 * they do in userspace.
 *
 * -------------------------------------------------------------------
 *
 * **Read-Write spin lock implementations.**
 *
 *  Ref: http://locklessinc.com/articles/locks
 *
 *  Both locks here are faster than pthread_rwlock and have very low
 *  overhead (usually 20-30ns).  They don't use any system mutexes and
 *  are very compact (4/8 bytes), so are suitable for per-instance
 *  based locking, particularly when contention is not expected.
 *
 *  For a spinlock, RWSpinLock is a reasonable choice.  (See the note
 *  about for why a spin lock is frequently a bad idea generally.)
 *  RWSpinLock has minimal overhead, and comparable contention
 *  performance when the number of competing threads is less than or
 *  equal to the number of logical CPUs.  Even as the number of
 *  threads gets larger, RWSpinLock can still be very competitive in
 *  READ, although it is slower on WRITE, and also inherently unfair
 *  to writers.
 *
 *  The lock will not grant any new shared (read) accesses while a thread
 *  attempting to acquire the lock in write mode is blocked. (That is,
 *  if the lock is held in shared mode by N threads, and a thread attempts
 *  to acquire it in write mode, no one else can acquire it in shared mode
 *  until these N threads release the lock and then the blocked thread
 *  acquires and releases the exclusive lock.) This also applies for
 *  attempts to reacquire the lock in shared mode by threads that already
 *  hold it in shared mode, making the lock non-reentrant.
 *
 *  RWSpinLock handles 2^30 - 1 concurrent readers.
 *
 * -------------------------------------------------------------------
 *
 * **Benchmark on (Intel(R) Xeon(R) CPU L5630 @ 2.13GHz) 8 cores(16 HTs)**
 *
 * 1. Single thread benchmark (read/write lock + unlock overhead)
 * | Benchmark                     | Iters  | Total t  | t/iter   | iter/sec  |
 * | ----------------------------- | :----: | :------: | :------: | :-------: |
 * | *      BM_RWSpinLockRead      | 100000 | 1.786 ms | 17.86 ns |    53.4 M |
 * | +30.5% BM_RWSpinLockWrite     | 100000 | 2.331 ms | 23.31 ns |   40.91 M |
 * | + 175% BM_PThreadRWMutexRead  | 100000 | 4.917 ms | 49.17 ns |    19.4 M |
 * | + 166% BM_PThreadRWMutexWrite | 100000 | 4.757 ms | 47.57 ns |   20.05 M |
 *
 * 2. Contention Benchmark (90% read, 10% write)
 * <table>
 *  <tr>
 *   <th>Benchmark</th> <th>hits</th> <th>average</th>
 *   <th>min</th> <th>max</th> <th>sigma</th>
 *  </tr>
 *  <tr>
 *   <th colspan="6">8 threads</th>
 *  </tr>
 *  <tr>
 *   <td>RWSpinLock Write</td> <td>142666</td> <td>220 ns</td>
 *   <td>78 ns</td> <td>40.8 us</td> <td>269 ns</td>
 *  </tr>
 *  <tr>
 *   <td>RWSpinLock Read</td> <td>1282297</td> <td>222 ns</td>
 *   <td>80 ns</td> <td>37.7 us</td> <td>248 ns</td>
 *  </tr>
 *  <tr>
 *   <td>pthread_rwlock_t Write</td> <td>84248</td> <td>2.48 us</td>
 *   <td>99 ns</td> <td>269 us</td> <td>8.19 us</td>
 *  </tr>
 *  <tr>
 *   <td>pthread_rwlock_t Read</td> <td>761646</td> <td>933 ns</td>
 *   <td>101 ns</td> <td>374 us</td> <td>3.25 us</td>
 *  </tr>
 *  <tr>
 *   <th colspan="6">16 threads</th>
 *  </tr>
 *  <tr>
 *   <td>RWSpinLock Write</td> <td>124236</td> <td>237 ns</td>
 *   <td>78 ns</td> <td>261 us</td> <td>801 ns</td>
 *  </tr>
 *  <tr>
 *   <td>RWSpinLock Read</td> <td>1115807</td> <td>236 ns</td>
 *   <td>78 ns</td> <td>2.27 ms</td> <td>2.17 us</td>
 *  </tr>
 *  <tr>
 *   <td>pthread_rwlock_t Write</td> <td>83363</td> <td>7.12 us</td>
 *   <td>99 ns</td> <td>785 us</td> <td>28.1 us</td>
 *  </tr>
 *  <tr>
 *   <td>pthread_rwlock_t Read</td> <td>754978</td> <td>2.18 us</td>
 *   <td>101 ns</td> <td>1.02 ms</td> <td>14.3 us</td>
 *  </tr>
 *  <tr>
 *   <th colspan="6">50 threads</th>
 *  </tr>
 *  <tr>
 *   <td>RWSpinLock Write</td> <td>131142</td> <td>1.37 us</td>
 *   <td>82 ns</td> <td>7.53 ms</td> <td>68.2 us</td>
 *  </tr>
 *  <tr>
 *   <td>RWSpinLock Read</td> <td>1181240</td> <td>262 ns</td>
 *   <td>78 ns</td> <td>6.62 ms</td> <td>12.7 us</td>
 *  </tr>
 *  <tr>
 *   <td>pthread_rwlock_t Write</td> <td>80849</td> <td>112 us</td>
 *   <td>103 ns</td> <td>4.52 ms</td> <td>263 us</td>
 *  </tr>
 *  <tr>
 *   <td>pthread_rwlock_t Read</td> <td>728698</td> <td>24 us</td>
 *   <td>101 ns</td> <td>7.28 ms</td> <td>194 us</td>
 *  </tr>
 * </table>
 */

#include <algorithm>
#include <atomic>
#include <thread>
#include "../Common.h"

UTILITIES_NAMESPACE_BEGIN

/**
 * \addtogroup MemorySafety
 * @{
 */
namespace Memory {
/**
 * \brief High-performance read-write-spinlock, see RWSpinLock.hpp for
 *        details.
 */
class RWSpinLock {
    enum : int32_t { READER = 4, UPGRADED = 2, WRITER = 1 };

public:
    constexpr RWSpinLock() : bits_(0) {}

    RWSpinLock(RWSpinLock const&) = delete;
    RWSpinLock& operator=(RWSpinLock const&) = delete;

    /** \brief Lockable Concept */
    void lock() {
        uint_fast32_t count = 0;
        while (!try_lock()) {
            if (++count > 1000) {
                std::this_thread::yield();
            }
        }
    }

    /** \brief Writer is responsible for clearing up both the UPGRADED and WRITER bits. */
    void unlock() {
        static_assert(READER > WRITER + UPGRADED, "wrong bits!");
        bits_.fetch_and(~(WRITER | UPGRADED), std::memory_order_release);
    }

    /** \brief SharedLockable Concept */
    void lock_shared() {
        uint_fast32_t count = 0;
        while (!try_lock_shared()) {
            if (++count > 1000) {
                std::this_thread::yield();
            }
        }
    }

    void unlock_shared() {
        bits_.fetch_add(-READER, std::memory_order_release);
    }

    /** \brief Downgrade the lock from writer status to reader status. */
    void unlock_and_lock_shared() {
        bits_.fetch_add(READER, std::memory_order_acquire);
        unlock();
    }

    /** \brief UpgradeLockable Concept */
    void lock_upgrade() {
        uint_fast32_t count = 0;
        while (!try_lock_upgrade()) {
            if (++count > 1000) {
                std::this_thread::yield();
            }
        }
    }

    void unlock_upgrade() {
        bits_.fetch_add(-UPGRADED, std::memory_order_acq_rel);
    }

    /** \brief unlock upgrade and try to acquire write lock */
    void unlock_upgrade_and_lock() {
        int64_t count = 0;
        while (!try_unlock_upgrade_and_lock()) {
            if (++count > 1000) {
                std::this_thread::yield();
            }
        }
    }

    /**\brief  unlock upgrade and read lock atomically */
    void unlock_upgrade_and_lock_shared() {
        bits_.fetch_add(READER - UPGRADED, std::memory_order_acq_rel);
    }

    /** \brief write unlock and upgrade lock atomically */
    void unlock_and_lock_upgrade() {
        // need to do it in two steps here -- as the UPGRADED bit might be OR-ed
        // at the same time when other threads are trying do try_lock_upgrade().
        bits_.fetch_or(UPGRADED, std::memory_order_acquire);
        bits_.fetch_add(-WRITER, std::memory_order_release);
    }

    /** Attempt to acquire writer permission. Return false if we didn't get it. */
    bool try_lock() {
        int32_t expect = 0;
        return bits_.compare_exchange_strong(
                    expect, WRITER, std::memory_order_acq_rel);
    }

    /**
     * \brief Try to get reader permission on the lock. This can fail if we
     *        find out someone is a writer or upgrader.
     * \details
     *   Setting the UPGRADED bit would allow a writer-to-be to indicate
     *   its intention to write and block any new readers while waiting
     *   for existing readers to finish and release their read locks. This
     *   helps avoid starving writers (promoted from upgraders).
     */
    bool try_lock_shared() {
        // fetch_add is considerably (100%) faster than compare_exchange,
        // so here we are optimizing for the common (lock success) case.
        int32_t value = bits_.fetch_add(READER, std::memory_order_acquire);
        if (value & (WRITER | UPGRADED)) {
            bits_.fetch_add(-READER, std::memory_order_release);
            return false;
        }
        return true;
    }

    /** \brief try to unlock upgrade and write lock atomically */
    bool try_unlock_upgrade_and_lock() {
        int32_t expect = UPGRADED;
        return bits_.compare_exchange_strong(
                    expect, WRITER, std::memory_order_acq_rel);
    }

    /**
     * \brief try to acquire an upgradable lock.
     * \note
     *   when failed, we cannot flip the UPGRADED bit back,
     *   as in this case there is either another upgrade lock or a write lock.
     *   If it's a write lock, the bit will get cleared up when that lock's done
     *   with unlock().
     */
    bool try_lock_upgrade() {
        int32_t value = bits_.fetch_or(UPGRADED, std::memory_order_acquire);

        return ((value & (UPGRADED | WRITER)) == 0);
    }

    /** \brief mainly for debugging purposes. */
    int32_t bits() const {
        return bits_.load(std::memory_order_acquire);
    }

    class ReadHolder;
    class UpgradedHolder;
    class WriteHolder;

    /**
     * \brief RAII guard for read lock with RWSpinLock::lock_shared() on
     *        construction and RWSpinLock::unlock_shared() on destruction.
     */
    class ReadHolder {
    public:
        explicit ReadHolder(RWSpinLock* lock) : lock_(lock) {
            if (lock_) {
                lock_->lock_shared();
            }
        }

        explicit ReadHolder(RWSpinLock& lock) : lock_(&lock) {
            lock_->lock_shared();
        }

        ReadHolder(ReadHolder&& other) noexcept : lock_(other.lock_) {
            other.lock_ = nullptr;
        }

        /** \brief down-grade */
        explicit ReadHolder(UpgradedHolder&& upgraded) : lock_(upgraded.lock_) {
            upgraded.lock_ = nullptr;
            if (lock_) {
                lock_->unlock_upgrade_and_lock_shared();
            }
        }

        explicit ReadHolder(WriteHolder&& writer) : lock_(writer.lock_) {
            writer.lock_ = nullptr;
            if (lock_) {
                lock_->unlock_and_lock_shared();
            }
        }

        ReadHolder& operator=(ReadHolder&& other) {
            using std::swap;
            swap(lock_, other.lock_);
            return *this;
        }

        ReadHolder(const ReadHolder& other) = delete;
        ReadHolder& operator=(const ReadHolder& other) = delete;

        ~ReadHolder() {
            if (lock_) {
                lock_->unlock_shared();
            }
        }

        void reset(RWSpinLock* lock = nullptr) {
            if (lock == lock_) {
                return;
            }
            if (lock_) {
                lock_->unlock_shared();
            }
            lock_ = lock;
            if (lock_) {
                lock_->lock_shared();
            }
        }

        void swap(ReadHolder& other) {
            std::swap(lock_, other.lock_);
        }

    private:
        friend class UpgradedHolder;
        friend class WriteHolder;
        RWSpinLock* lock_;
    };

    /**
     * \brief RAII guard for upgrade lock with RWSpinLock::lock_upgrade() on
     *        construction and RWSpinLock::unlock_upgrade() on destruction.
     */
    class UpgradedHolder {
    public:
        explicit UpgradedHolder(RWSpinLock* lock) : lock_(lock) {
            if (lock_) {
                lock_->lock_upgrade();
            }
        }

        explicit UpgradedHolder(RWSpinLock& lock) : lock_(&lock) {
            lock_->lock_upgrade();
        }

        explicit UpgradedHolder(WriteHolder&& writer) {
            lock_ = writer.lock_;
            writer.lock_ = nullptr;
            if (lock_) {
                lock_->unlock_and_lock_upgrade();
            }
        }

        UpgradedHolder(UpgradedHolder&& other) noexcept : lock_(other.lock_) {
            other.lock_ = nullptr;
        }

        UpgradedHolder& operator=(UpgradedHolder&& other) {
            using std::swap;
            swap(lock_, other.lock_);
            return *this;
        }

        UpgradedHolder(const UpgradedHolder& other) = delete;
        UpgradedHolder& operator=(const UpgradedHolder& other) = delete;

        ~UpgradedHolder() {
            if (lock_) {
                lock_->unlock_upgrade();
            }
        }

        void reset(RWSpinLock* lock = nullptr) {
            if (lock == lock_) {
                return;
            }
            if (lock_) {
                lock_->unlock_upgrade();
            }
            lock_ = lock;
            if (lock_) {
                lock_->lock_upgrade();
            }
        }

        void swap(UpgradedHolder& other) {
            using std::swap;
            swap(lock_, other.lock_);
        }

    private:
        friend class WriteHolder;
        friend class ReadHolder;
        RWSpinLock* lock_;
    };

    /**
     * \brief RAII guard for write lock with RWSpinLock::lock() on
     *        construction and RWSpinLock::unlock() on destruction.
     */
    class WriteHolder {
    public:
        explicit WriteHolder(RWSpinLock* lock) : lock_(lock) {
            if (lock_) {
                lock_->lock();
            }
        }

        explicit WriteHolder(RWSpinLock& lock) : lock_(&lock) {
            lock_->lock();
        }

        /** \brief promoted from an upgrade lock holder */
        explicit WriteHolder(UpgradedHolder&& upgraded) {
            lock_ = upgraded.lock_;
            upgraded.lock_ = nullptr;
            if (lock_) {
                lock_->unlock_upgrade_and_lock();
            }
        }

        WriteHolder(WriteHolder&& other) noexcept : lock_(other.lock_) {
            other.lock_ = nullptr;
        }

        WriteHolder& operator=(WriteHolder&& other) {
            using std::swap;
            swap(lock_, other.lock_);
            return *this;
        }

        WriteHolder(const WriteHolder& other) = delete;
        WriteHolder& operator=(const WriteHolder& other) = delete;

        ~WriteHolder() {
            if (lock_) {
                lock_->unlock();
            }
        }

        void reset(RWSpinLock* lock = nullptr) {
            if (lock == lock_) {
                return;
            }
            if (lock_) {
                lock_->unlock();
            }
            lock_ = lock;
            if (lock_) {
                lock_->lock();
            }
        }

        void swap(WriteHolder* other) {
            using std::swap;
            swap(lock_, other->lock_);
        }

    private:
        friend class ReadHolder;
        friend class UpgradedHolder;
        RWSpinLock* lock_;
    };

private:
    std::atomic<int32_t> bits_;
};
} // namespace Memory
/** @} */

UTILITIES_NAMESPACE_END

#endif  // CPP_UTILITIES_MEMORYSAFETY_RWSPINLOCK_HPP
