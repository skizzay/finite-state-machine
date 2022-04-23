#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_coordinator.h"
#include "skizzay/fsm/transition_table.h"

#include <exception>
#include <iostream>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {

namespace event_transition_context_details_ {
} // namespace event_transition_context_details_

template <concepts::event Event, concepts::event_engine EventEngine,
          concepts::state_provider StateProvider,
          concepts::transition_table TransitionTable>
struct event_context_root {
  using event_type = Event;
  using events_list_type = events_list_t<EventEngine>;
  using states_list_type = states_list_t<StateProvider>;
  using transition_table_type = TransitionTable;

  constexpr explicit event_context_root(
      event_type const &event,
      TransitionCoordinator &transition_coordinator) noexcept
      : event_{event}, transition_coordinator_{transition_coordinator} {}

  constexpr event_type const &event() const noexcept { return event_; }

  constexpr void
  on_transition(concepts::transition_in<transition_table_type> auto &) {
    std::cerr << "Root of FSM attempting to transition to a state outside of "
                 "the FSM.";
    std::terminate();
  }

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &event) {
    transition_coordinator_.post_event(event);
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return transition_coordinator_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return transition_coordinator_.template state<State>();
  }

  constexpr concepts::transition_table auto
  get_transitions(concepts::state_in<current_states_list_t<
                      transition_table_type>> auto const &state) noexcept {
    return get_transition_table_for_current_state(transition_table_, state,
                                                  event());
  }

  constexpr std::tuple<>
  get_transitions(concepts::state auto const &) noexcept {
    return {};
  }

  template <concepts::state_in<next_states_list_type> State>
  constexpr void schedule_entry() noexcept {
    // entry_coordinator_.template schedule_entry<State>();
  }

private:
  event_type const &event_;
  TransitionCoordinator &transition_coordinator_;
};
} // namespace skizzay::fsm
