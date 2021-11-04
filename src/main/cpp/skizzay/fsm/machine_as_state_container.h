#pragma once

#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/type_list.h>
#include <type_traits>

namespace skizzay::fsm {

namespace machine_as_state_container_details_ {

template <typename Machine> struct as_self_transition {
  template <typename Event>
  using type = simple_transition<Machine, Machine, Event>;
};

template <typename Machine>
using self_transition_table_for_each_event_t =
    as_container_t<map_t<events_list_t<Machine>,
                         typename as_self_transition<Machine>::template type>,
                   std::tuple>;

template <concepts::machine Machine> class container {
  Machine machine_;

public:
  using states_list_type = states_list<Machine>;
  using self_transition_table_for_each_event_type =
      self_transition_table_for_each_event_t<Machine>;

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
    if (machine_.is_running()) {
      machine_.stop();
    }
  }

  template <concepts::state PreviousState, std::same_as<Machine>,
            concepts::machine ParentMachine, concepts::event Event>
  void on_entry(ParentMachine &, Event const &event) {
    if (machine_.is_stopped()) {
      machine_.start();
    }
    machine_.on(event);
  }

  template <std::same_as<Machine>, concepts::machine ParentMachine,
            concepts::event Event>
  void on_reentry(ParentMachine &, Event const &event) {
    machine_.on(event);
  }

  template <std::same_as<Machine>,
            concepts::state_in<states_list_type> NextState,
            concepts::machine ParentMachine, concepts::event Event>
  void on_exit(ParentMachine &, Event const &event) {
    machine_.on(event);
    machine_.stop();
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