#pragma once

#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {

namespace machine_as_state_container_details_ {
template <concepts::machine Machine> class container {
  Machine machine_;

public:
  using states_list_type = states_list<Machine>;

  template <concepts::machine Machine2>
  constexpr explicit container(Machine2 &&machine) noexcept(
      std::is_nothrow_constructible_v<Machine, Machine2>)
      : machine_{(Machine2 &&) machine} {}

  template <concepts::state_in<states_list_type> State>
  constexpr bool is() const noexcept {
    return machine_.template is<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr optional_reference<State const> current_state() const noexcept {
    return machine_.template current_state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &any_state() noexcept {
    return machine_.template any_state<State>();
  }

  template <concepts::state_in<states_list_type> State,
            concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return machine_.template ancestry_to<State>(ancestry);
  }

  template <concepts::machine ParentMachine>
  constexpr void on_initial_entry(ParentMachine &) {
    // TODO: Determine if we need to pass in the parent machine
    machine_.start();
  }

  template <concepts::machine ParentMachine>
  constexpr void on_final_exit(ParentMachine &) {
    // TODO: Determine if we need to pass in the parent machine
    machine_.stop();
  }

  template <concepts::state PreviousState,
            concepts::state_in<states_list_type> CurrentState,
            concepts::machine ParentMachine, concepts::event Event>
  void on_entry(ParentMachine &, Event const &) {
    if constexpr (concepts::event_in<Event, events_type_list>) {
      // TODO: Determine if we need to pass in the parent machine
      machine_.on_entry(event);
    }
  }

  template <concepts::state_in<states_list_type> CurrentState,
            concepts::machine ParentMachine, concepts::event Event>
  void on_reentry(ParentMachine &, Event const &) {
    if constexpr (concepts::event_in<Event, events_type_list>) {
      // TODO: Determine if we need to pass in the parent machine
      machine_.on_reentry(event);
    }
  }

  template <concepts::state PreviousState,
            concepts::state_in<states_list_type> CurrentState,
            concepts::machine ParentMachine, concepts::event Event>
  void on_exit(ParentMachine &, Event const &) {
    if constexpr (concepts::event_in<Event, events_type_list>) {
      // TODO: Determine if we need to pass in the parent machine
      machine_.on_exit(event);
    }
  }

  template <concepts::machine ParentMachine,
            concepts::transition_coordinator TransitionCoordinator>
  bool schedule_acceptable_transitions(ParentMachine const &,
                                       TransitionCoordinator &) const {
    // TODO: Figure out something better than stubbing
    return false;
  }
};
} // namespace machine_as_state_container_details_

template <concepts::machine Machine>
using machine_as_state_container =
    machine_as_state_container_details_::container<Machine>;
} // namespace skizzay::fsm