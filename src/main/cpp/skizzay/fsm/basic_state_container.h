#pragma once

#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/enter.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/exit.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/reenter.h>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/type_list.h>
#include <type_traits>

namespace skizzay::fsm {

namespace basic_state_container_details_ {

template <concepts::state State> struct container {
  using states_list_type = states_list<State>;
  State current_state_;

  constexpr container() noexcept(
      std::is_nothrow_default_constructible_v<State>) requires
      std::is_default_constructible_v<State>
  = default;

  template <typename... Args>
  requires std::is_constructible_v<State, Args...>
  constexpr explicit container(Args &&...args) noexcept(
      std::is_nothrow_constructible_v<State, Args...>)
      : current_state_{std::forward<Args>(args)...} {}

  template <concepts::state S> constexpr bool is() const noexcept {
    return std::is_same_v<State, S>;
  }

  template <std::same_as<State>>
  constexpr optional_reference<State const> current_state() const noexcept {
    return current_state_;
  }

  template <std::same_as<State>> constexpr State &any_state() noexcept {
    return current_state_;
  }

  template <std::same_as<State>, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return ancestry.with_new_generation(current_state_);
  }

  template <concepts::machine Machine>
  constexpr void on_initial_entry(Machine &machine) {
    enter(current_state_, machine, initial_entry_event);
  }

  template <concepts::machine Machine>
  constexpr void on_final_exit(Machine &machine) {
    exit(current_state_, machine, final_exit_event);
  }

  template <concepts::state PreviousState, std::same_as<State>,
            concepts::machine Machine, concepts::event Event>
  constexpr void on_entry(Machine &machine, Event const &event) {
    enter(current_state_, machine, event);
  }

  template <std::same_as<State>, concepts::machine Machine,
            concepts::event Event>
  constexpr void on_reentry(Machine &machine, Event const &event) {
    reenter(current_state_, machine, event);
  }

  template <std::same_as<State>, concepts::state NextState,
            concepts::machine Machine, concepts::event Event>
  constexpr void on_exit(Machine &machine, Event const &event) {
    exit(current_state_, machine, event);
  }

  template <concepts::machine Machine,
            concepts::transition_coordinator TransitionCoordinator>
  constexpr bool
  schedule_acceptable_transitions(Machine const &machine,
                                  TransitionCoordinator &coordinator) const {
    return coordinator.schedule_accepted_transitions(current_state_, machine);
  }
};
} // namespace basic_state_container_details_

template <typename State>
using basic_state_container = basic_state_container_details_::container<State>;
} // namespace skizzay::fsm