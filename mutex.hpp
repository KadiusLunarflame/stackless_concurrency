#pragma once

#include <atomic.hpp>
#include <experimental/coroutine>

// std::unique_lock
#include <mutex>

#include <cassert>
#include <cstdint>

class Mutex {
  using UniqueLock = std::unique_lock<Mutex>;

  struct [[nodiscard]] Locker {
    Mutex& mutex_;
    std::experimental::coroutine_handle<> handle_;
    Locker* next_{nullptr};

    explicit Locker(Mutex& mutex) : mutex_(mutex) {
    }

    bool await_ready() {
      return mutex_.TryLock();
    }

    // NOLINTNEXTLINE
    bool await_suspend(std::experimental::coroutine_handle<> handle) {
      handle_ = handle;
      return mutex_.TryLockOrEnqueue(this);
    }

    // NOLINTNEXTLINE
    UniqueLock await_resume() {
      return std::unique_lock(mutex_, std::adopt_lock);
    }
  };
//  friend struct Locker;
  Locker* empty = reinterpret_cast<Locker*>((std::uintptr_t(-1)));

 public:
  ~Mutex() {
    assert(head_ == empty);
  }

  // Asynchronous
  auto ScopedLock() {
    return Locker{*this};
  }

  bool TryLock() {
    return claim();
  }

  // For std::unique_lock
  // Do not use directly
  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  // returns false if lock acquired
  // returns true if enqueued
  //to enqueue an awaiter means to weave into the lock-free stack of awaiters
  bool TryLockOrEnqueue(Locker* locker) {
////this works!!!
      for (;;) {
          ////attempt to claim mutex
          ////if done -> return false(!)
          Locker *old = empty;
          if (head_.compare_exchange_weak(old, nullptr)) {
              return false;
          }

          ////lock-free push()
          locker->next_ = old;
          if (head_.compare_exchange_weak(locker->next_, locker)) {
              return true;  ////if successfully pushed awaiter onto stack then suspend coroutine and wait
          }
      }
////
  }

  //no concurrency in this call
  void Unlock() {
/////////////////////////////////////////////////
//    assert(head_ != empty);
//    Locker* old = nullptr;
//    if(head_.compare_exchange_strong(old, empty)) {
//      return;
//    }
    if (relinquish()) {
      return;
    }

    auto next_owner = pop();//no concurrency here
    next_owner->handle_.resume();
//////////////////////////////////////////////////
  }
 private:
  ////implemented with a lock-free guarantee, which is actually not needed for mutex, but... :)
  Locker* pop() {
    for(;;) {
      Locker* cur_head = head_.load();
      ////no such scenario:
//      if(cur_head == empty) {
//        return empty;
//      }

      ////no such scenario:
//      if(cur_head == nullptr) {
//        return nullptr;
//      }
      ////just pop
      if(head_.compare_exchange_weak(cur_head, cur_head->next_)) {
        return cur_head;
      }
    }
  }

  bool
  relinquish() {
    Locker* old = nullptr;
    return head_.compare_exchange_strong(old, empty);
  }

  bool
  claim() {
    Locker* old = empty;
    return head_.compare_exchange_weak(old, nullptr);
  }


 private:
  std::atomic<Locker*> head_{empty};
};//mutex
//////////////////////////////////////////////////////////////////////////////////
