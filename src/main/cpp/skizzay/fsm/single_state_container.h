#pragma once

#include "skizzay/fsm/basic_state_container.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/states_list.h"

#include <type_traits>

namespace skizzay::fsm {

template <concepts::state State>
requires(!concepts::state_container<State>) struct single_state
    : basic_state_container<single_state<State>> {
  using states_list_type = states_list<State>;

  constexpr single_state() noexcept(
      std::is_nothrow_default_constructible_v<
          State>) requires(std::is_default_constructible_v<State>) = default;

  template <typename... Args>
  requires std::is_constructible_v<State, Args...>
  constexpr explicit single_state(Args &&...args) noexcept(
      std::is_nothrow_constructible_v<State, Args...>)
      : basic_state_container<single_state<State>>{}, state_{std::forward<Args>(
                                                          args)...} {}

private:
  friend basic_state_container<single_state<State>>;
  State state_;

  constexpr State &get_state() noexcept { return state_; }

  constexpr State const &get_state() const noexcept { return state_; }
};

template <concepts::state State>
single_state(State &&) -> single_state<std::remove_cvref_t<State>>;
} // namespace skizzay::fsm