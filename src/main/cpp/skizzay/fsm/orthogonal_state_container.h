#pragma once

#include <skizzay/fsm/concepts.h>
#include <tuple>

namespace skizzay::fsm {

namespace orthogoanl_state_container_details_ {

template <concepts::state_container... StateContainers> class container {
  using tuple_type = std::tuple<StateContainers...>;
  tuple_type state_containers_;

public:
  using states_list_type = concat_t<states_list_t<StateContainers>...>;

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template is<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template current_state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &any_state() noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template any_state<S>();
  }

  template <concepts::state_in<states_list_type> S, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template ancestry_to<S>(ancestry);
  }

  template <concepts::machine Machine>
  constexpr void on_initial_entry(Machine &machine) {
    std::apply(
        [&machine](StateContainers const &...state_containers) {
          (state_containers.on_initial_entry(machine), ...);
        },
        state_containers_);
  }

  template <concepts::machine Machine>
  constexpr void on_final_exit(Machine &machine) {
    std::apply(
        [&machine](StateContainers const &...state_containers) {
          (state_containers.on_final_exit(machine), ...);
        },
        state_containers_);
  }

  template <concepts::state PreviousState,
            concepts::state_in<states_list_type> CurrentState,
            concepts::machine Machine, concepts::event Event>
  constexpr void on_entry(Machine &machine, Event const &event) {
    std::get<state_container_for_t<tuple_type, CurrentState>>(state_containers_)
        .template on_entry<PreviousState, CurrentState>(machine, event);
  }

  template <concepts::state_in<states_list_type> State,
            concepts::machine Machine, concepts::event Event>
  constexpr void on_reentry(Machine &machine, Event const &event) {
    std::get<state_container_for_t<tuple_type, State>>(state_containers_)
        .template on_reentry<State>(machine, event);
  }

  template <concepts::state_in<states_list_type> CurrentState,
            concepts::state NextState, concepts::machine Machine,
            concepts::event Event>
  constexpr void on_exit(Machine &machine, Event const &event) {
    std::get<state_container_for_t<tuple_type, CurrentState>>(state_containers_)
        .template on_exit<CurrentState, NextState>(machine, event);
  }

  template <concepts::machine Machine,
            concepts::transition_coordinator TransitionCoordinator>
  constexpr bool
  schedule_acceptable_transitions(Machine const &machine,
                                  TransitionCoordinator &coordinator) const {
    using current_transition_states =
        typename TransitionCoordinator::current_states_list_type;
    if constexpr (contains_any<states_list_type,
                               current_transition_states>::value) {
      return std::apply(
          [](auto... results) noexcept { return (results || ... || false); },
          std::tuple{std::get<StateContainers>(machine, coordinator)...});
    } else {
      return false;
    }
  }
};
} // namespace orthogoanl_state_container_details_

template <concepts::state_container... StateContainers>
using orthogonal_state_container =
    orthogonal_state_container_details_::container<StateContainers...>;
} // namespace skizzay::fsm