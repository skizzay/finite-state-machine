#pragma once

#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {

namespace node_transition_coordinator_details_ {
template <typename StateContainer, typename ParentTransitionCoordinator>
struct impl {
  using transition_table_type = transition_table_t<ParentTransitionCoordinator>;
 
  constexpr explicit impl(
      ParentTransitionCoordinator &parent_transition_coordinator,
      StateContainer &container) noexcept
      : parent_{parent_transition_coordinator}, container_{container} {}

  template <concepts::transition Transition, concepts::machine Machine>
  constexpr void on_transition(Transition &transition, Machine &machine,
                               event_t<Transition> const &event) {
    scheduled_ = true;
    if constexpr (contains_v<states_list_t<StateContainer>,
                             next_state_t<Transition>>) {
      this->template schedule_entry<next_state_t<Transition>>();

    } else {
      exiting_container_ = true;
      parent_.on_transition(transition, machine, event);
    }
  }

  template <concepts::state State> constexpr void schedule_entry() noexcept {
    parent_.template schedule_entry<State>();
  }

  template <concepts::state State>
  constexpr decltype(auto) get_transitions(State const &state) {
    return parent_.get_transitions(state);
  }

  constexpr bool has_been_scheduled() const noexcept { return scheduled_; }

  constexpr bool will_exit_container() const noexcept {
    return exiting_container_;
  }

private:
  ParentTransitionCoordinator &parent_;
  StateContainer &container_;
  bool scheduled_ = false;
  bool exiting_container_ = false;
};
} // namespace node_transition_coordinator_details_

template <concepts::state_container StateContainer,
          concepts::transition_coordinator ParentTransitionCoordinator>
using node_transition_coordinator =
    node_transition_coordinator_details_::impl<StateContainer,
                                               ParentTransitionCoordinator>;
} // namespace skizzay::fsm