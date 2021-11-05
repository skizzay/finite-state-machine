#pragma once

#include <bitset>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/entry_coordinator.h>
#include <skizzay/fsm/trigger.h>
#include <skizzay/fsm/type_list.h>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {

namespace transition_coordinator_details_ {

template <concepts::event Event> struct has_compatible_event {
  template <concepts::transition Transition>
  using test = std::is_convertible<std::remove_cvref_t<Event> const &,
                                   event_t<Transition> const &>;
};

template <typename TransitionTable, typename Event>
using candidate_transition_table_t =
    filter_t<TransitionTable, has_compatible_event<Event>::template test>;

template <typename TransitionTable>
using candidate_entry_states_list_t = as_container_t<
    unique_t<map_t<
        filter_t<TransitionTable, negate<is_self_transition>::template apply>,
        next_state_t>>,
    states_list>;

template <typename State> struct is_current_state {
  template <typename Transition>
  using for_transition = std::is_same<State, current_state_t<Transition>>;
};

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
struct impl {
  using transition_table_type =
      candidate_transition_table_t<transition_table_t<Machine>, Event>;

  constexpr explicit impl(
      transition_table_t<Machine> &transition_table) noexcept
      : transition_table_{transition_table} {}

  template <concepts::transition_in<transition_table_type> Transition>
  [[noreturn]] constexpr void on_transition(Transition &, Machine &,
                                            Event const &) {
    std::cerr << "Root of FSM attempting to transition to a state outside of "
                 "the FSM.";
    std::terminate();
  }

  template <concepts::state_in<next_states_list_t<transition_table_type>> State>
  constexpr void schedule_entry() noexcept {
    entry_coordinator_.template schedule_entry<State>();
  }

  template <
      concepts::state_in<current_states_list_t<transition_table_type>> State>
  constexpr auto get_transitions(State const &) noexcept {
    using transitions_for_state_type = as_container_t<
        filter_t<transition_table_type,
                 is_current_state<State>::template for_transition>,
        transitions_list>;
    return []<template <typename...> typename Template,
              concepts::transition... Transitions>(
        transition_table_t<Machine> & transition_table,
        Template<Transitions...> const) noexcept {
      return std::tie((std::get<Transitions>(transition_table), ...));
    }
    (transition_table_, transitions_for_state_type{});
  }

  constexpr std::tuple<>
  get_transitions(concepts::state auto const &) noexcept {
    return {};
  }

  constexpr auto const &entries() const noexcept { return entry_coordinator_; }

private:
  transition_table_t<Machine> &transition_table_;
  entry_coordinator<next_states_list_t<transition_table_type>>
      entry_coordinator_;
};

} // namespace transition_coordinator_details_

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
using transition_coordinator =
    transition_coordinator_details_::impl<Machine, Event>;
} // namespace skizzay::fsm