#pragma once

#include <concepts>
#include <functional>
#include <queue>

namespace skizzay::fsm {

namespace internal_event_executor_details_ {
struct impl {
  template <std::invocable F> void schedule_execution(F &&f) {
    q_.emplace(std::forward<F>(f));
  }

  void process_internal_events() {
    while (!empty()) {
      std::function<void()> const callback{std::move(q_.front())};
      q_.pop();
      callback();
    }
  }

  bool empty() const noexcept { return std::empty(q_); }

private:
  std::queue<std::function<void()>> q_;
};
} // namespace internal_event_executor_details_

using internal_event_executor = internal_event_executor_details_::impl;

} // namespace skizzay::fsm