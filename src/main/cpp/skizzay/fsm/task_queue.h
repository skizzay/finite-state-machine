#pragma once

#include <functional>
#include <queue>

namespace skizzay::fsm {
struct task_queue {
  constexpr void push(std::invocable auto &&task) {
    tasks_.emplace(std::forward<decltype(task)>(task));
  }

  void run() {
    while (!std::empty(tasks_)) {
      auto const task{std::move(tasks_.front())};
      tasks_.pop();
      task();
    }
  }

private:
  std::queue<std::function<void()>> tasks_;
};
} // namespace skizzay::fsm