#pragma once

#include <skizzay/fsm/basic_state_container.h>
#include <skizzay/fsm/concepts.h>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace hierarchical_state_container_details_ {
enum class self_transition_behavior { internal, external };

template <self_transition_behavior SelfTransitionBehavior,
          concepts::state ParentState,
          concepts::state_container ChildStateContainer>
struct container {
  using states_list_type =
      concat_t<states_list<ParentState>, states_list_t<ChildStateContainer>>;

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    if constexpr (std::is_same_v<S, ParentState>) {
      return true;
    } else {
      return child_state_container_.template is<S>();
    }
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    if constexpr (std::is_same_v<S, ParentState>) {
      return parent_state_container_.template current_state<S>();
    } else {
      return child_state_container_.template current_state<S>();
    }
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &any_state() noexcept {
    if constexpr (std::is_same_v<S, ParentState>) {
      return parent_state_container_.template any_state<S>();
    } else {
      return child_state_container_.template any_state<S>();
    }
  }

  template <concepts::state_in<states_list_type> S, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    if constexpr (std::is_same_v<S, ParentState>) {
      return parent_state_container_.template ancestry_to<S>(ancestry);
    } else {
      return child_state_container_.template ancestry_to<S>(
          parent_state_container_.template ancestry_to<ParentState>(ancestry));
    }
  }

  template <concepts::machine Machine>
  constexpr void on_initial_entry(Machine &machine) {
    parent_state_container_.on_initial_entry(machine);
    child_state_container_.on_initial_entry(machine);
  }

  template <concepts::machine Machine>
  constexpr void on_final_exit(Machine &machine) {
    child_state_container_.on_final_exit(machine);
    parent_state_container_.on_final_exit(machine);
  }

  template <concepts::state PreviousState,
            concepts::state_in<states_list_type> CurrentState,
            concepts::machine Machine, concepts::event Event>
  constexpr void on_entry(Machine &machine, Event const &event) {
    if constexpr (std::is_same_v<CurrentState, ParentState>) {
      parent_state_container_.template on_entry(machine, event);
      child_state_container_.on_initial_entry(machine);
    } else {
      if constexpr (!contains_v<states_list_type, PreviousState>) {
        parent_state_container_.on_initial_entry(machine);
      }
      child_state_container_.template on_entry<PreviousState, CurrentState>(
          machine, event);
    }
  }

  template <concepts::state_in<states_list_type> S, concepts::machine Machine,
            concepts::event Event>
  constexpr void on_reentry(Machine &machine, Event const &event) {
    if constexpr (std::is_same_v<ParentState>) {
      if constexpr (self_transition_behavior::external ==
                    SelfTransitionBehavior) {
        child_state_container_.on_final_exit(machine);
        parent_state_container_.template on_reentry<S>(machine, event);
        child_state_container_.on_initial_entry(machine);
      } else {
        parent_state_container_.template on_reentry<S>(machine, event);
      }
    } else {
      child_state_container_.template on_reentry<S>(machine, event);
    }
  }

  template <concepts::state_in<states_list_type> CurrentState,
            concepts::state NextState, concepts::machine Machine,
            concepts::event Event>
  constexpr void on_exit(Machine &machine, Event const &event) {
    if constexpr (std::is_same_v<ParentState, CurrentState>) {
      child_state_container_.on_final_exit(machine);
      parent_state_container_.template on_exit<CurrentState, NextState>(machine,
                                                                        event);
    } else {
      child_state_container_.template on_exit<CurrentState, NextState>(machine,
                                                                       event);
      if constexpr (!contains_v<states_list_t<ChildStateContainer>,
                                NextState>) {
        parent_state_container_.on_final_exit(machine);
      }
    }
  }

  template <concepts::machine Machine,
            concepts::transition_coordinator TransitionCoordinator>
  constexpr bool
  schedule_acceptable_transitions(Machine const &machine,
                                  TransitionCoordinator &coordinator) const {
    return child_state_container_.schedule_acceptable_transitions(
               machine, coordinator) ||
           parent_state_container_.schedule_acceptable_transitions(machine,
                                                                   coordinator);
  }

private:
  basic_state_container<ParentState> parent_state_container_;
  ChildStateContainer child_state_container_;
};
} // namespace hierarchical_state_container_details_

template <concepts::state ParentState,
          concepts::state_container ChildStateContainer>
using parent_state_with_internal_self_transitions =
    hierarchical_state_container_details_::container<
        hierarchical_state_container_details_::self_transition_behavior::
            internal,
        ParentState, ChildStateContainer>;

template <concepts::state ParentState,
          concepts::state_container ChildStateContainer>
using parent_state_with_external_self_transitions =
    hierarchical_state_container_details_::container<
        hierarchical_state_container_details_::self_transition_behavior::
            external,
        ParentState, ChildStateContainer>;

} // namespace skizzay::fsm