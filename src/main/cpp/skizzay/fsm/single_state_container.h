#pragma once

#include <skizzay/fsm/basic_state_container.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/enter.h>
#include <skizzay/fsm/exit.h>
#include <skizzay/fsm/traits.h>
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

  constexpr State &get() noexcept { return state_; }

  constexpr State const &get() const noexcept { return state_; }

private:
  friend basic_state_container<single_state<State>>;
  State state_;

  template <concepts::machine_for<State> Machine, concepts::event Event>
  requires(
      concepts::event_in<Event, events_list_t<Machine>> ||
      std::same_as<
          Event,
          initial_entry_event_t>) constexpr void do_entry(Machine &machine,
                                                          Event const &event) {
    enter(state_, machine, event);
  }

  template <concepts::machine_for<State> Machine, concepts::event Event>
  requires(
      concepts::event_in<Event, events_list_t<Machine>> ||
      std::same_as<Event,
                   final_exit_event_t>) constexpr void do_exit(Machine &machine,
                                                               Event const
                                                                   &event) {
    exit(state_, machine, event);
  }
};

template <concepts::state State>
single_state(State &&) -> single_state<std::remove_cvref_t<State>>;
} // namespace skizzay::fsm