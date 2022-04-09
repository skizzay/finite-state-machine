#pragma once

#include <skizzay/fsm/basic_state_container.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/enter.h>
#include <skizzay/fsm/exit.h>
#include <skizzay/fsm/node_transition_coordinator.h>
#include <skizzay/fsm/reenter.h>
#include <skizzay/fsm/type_list.h>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace hierarchical_state_container_details_ {

template <concepts::state ParentState>
struct parent_state_container
    : basic_state_container<parent_state_container<ParentState>> {
  using states_list_type = states_list<ParentState>;
  ParentState state_;

  constexpr parent_state_container() noexcept(
      std::is_nothrow_default_constructible_v<ParentState>) = default;

  constexpr explicit parent_state_container(ParentState state) noexcept(
      std::is_nothrow_move_constructible_v<ParentState>)
      : basic_state_container<parent_state_container<ParentState>>{},
        state_{std::move_if_noexcept(state)} {}

  constexpr ParentState &get_state() noexcept { return state_; }

  constexpr ParentState const &get_state() const noexcept { return state_; }

  template <concepts::machine_for<ParentState> Machine, concepts::event Event>
  requires(
      concepts::event_in<Event, events_list_t<Machine>> ||
      std::same_as<
          Event,
          initial_entry_event_t>) constexpr void do_entry(Machine &machine,
                                                          Event const &event) {
    enter(state_, machine, event);
  }

  template <concepts::machine_for<ParentState> Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr void do_exit(Machine &, Event const &) noexcept {}

  template <concepts::machine_for<ParentState> Machine>
  constexpr void do_exit(Machine &machine, final_exit_event_t const) noexcept {
    exit(state_, machine, final_exit_event);
  }
};

template <typename ParentTransitionCoordinator, typename ParentStateContainer,
          typename ChildStateContainer>
struct child_transition_coordinator
    : node_transition_coordinator<ChildStateContainer,
                                  ParentTransitionCoordinator> {
  using transition_table_type = transition_table_t<ParentTransitionCoordinator>;

  constexpr explicit child_transition_coordinator(
      ParentTransitionCoordinator &parent_transition_coordinator,
      ParentStateContainer &parent_state_container,
      ChildStateContainer &child_state_container) noexcept
      : node_transition_coordinator<
            ChildStateContainer,
            ParentTransitionCoordinator>{parent_transition_coordinator,
                                         child_state_container},
        parent_state_container_{parent_state_container} {}

  using node_transition_coordinator<ChildStateContainer,
                                    ParentTransitionCoordinator>::on_transition;

  template <concepts::transition Transition, concepts::machine Machine>
  requires std::same_as<front_t<states_list_t<ParentStateContainer>>,
                        next_state_t<Transition>>
  constexpr void on_transition(Transition &, Machine &,
                               event_t<Transition> const &) {
    child_to_parent_transition_triggered_ = true;
  }

  constexpr bool child_to_parent_transition_triggered() const noexcept {
    return child_to_parent_transition_triggered_;
  }

private:
  ParentStateContainer &parent_state_container_;
  bool child_to_parent_transition_triggered_ = false;
};

template <concepts::state ParentState,
          concepts::state_container ChildStateContainer>
struct container {
  using states_list_type =
      concat_t<states_list<ParentState>, states_list_t<ChildStateContainer>>;

  constexpr container() noexcept(
      std::conjunction_v<
          std::is_nothrow_default_constructible<
              parent_state_container<ParentState>>,
          std::is_nothrow_default_constructible<ChildStateContainer>>) requires
      std::is_default_constructible_v<ParentState> &&
      std::is_default_constructible_v<ChildStateContainer>
      : parent_state_container_{}, child_state_container_{} {}

  constexpr explicit container(ParentState parent_state) noexcept(
      std::conjunction_v<
          std::is_nothrow_move_constructible<
              parent_state_container<ParentState>>,
          std::is_nothrow_default_constructible<ChildStateContainer>>)
      : parent_state_container_{std::move_if_noexcept(parent_state)},
        child_state_container_{} {}

  constexpr explicit container(ChildStateContainer child_state_container) noexcept(
      std::conjunction_v<
          std::is_nothrow_default_constructible<
              parent_state_container<ParentState>>,
          std::is_nothrow_move_constructible<ChildStateContainer>>)
      : parent_state_container_{}, child_state_container_{std::move_if_noexcept(
                                       child_state_container)} {}

  constexpr explicit container(
      ParentState parent_state,
      ChildStateContainer
          child_state_container) noexcept(std::
                                              conjunction_v<
                                                  std::is_nothrow_constructible<
                                                      parent_state_container<
                                                          ParentState>,
                                                      ParentState &&>,
                                                  std::
                                                      is_nothrow_move_constructible<
                                                          ChildStateContainer>>)
      : parent_state_container_{std::move_if_noexcept(parent_state)},
        child_state_container_{std::move_if_noexcept(child_state_container)} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    if constexpr (std::is_same_v<S, ParentState>) {
      return parent_state_container_.is_active();
    } else {
      return child_state_container_.template is<S>();
    }
  }

  constexpr bool is_active() const noexcept {
    return child_state_container_.is_active();
  }

  constexpr bool is_inactive() const noexcept {
    return parent_state_container_.is_inactive();
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
  constexpr S &get() noexcept {
    if constexpr (std::is_same_v<S, ParentState>) {
      return parent_state_container_.state_;
    } else {
      return child_state_container_.template get<S>();
    }
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S const &get() const noexcept {
    if constexpr (std::is_same_v<S, ParentState>) {
      return parent_state_container_.state_;
    } else {
      return child_state_container_.template get<S>();
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

  template <concepts::entry_coordinator EntryCoordinator,
            concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr void on_entry(EntryCoordinator const &entry_coordinator,
                          Machine &machine, Event const &event) {
    constexpr auto enter_container =
        []<typename StateContainer>(EntryCoordinator const &entry_coodinator,
                                    Machine &machine, Event const &event,
                                    StateContainer &container) {
          using applicable_state_containers_list_type =
              typename EntryCoordinator::applicable_state_containers_list_type<
                  state_containers_list<StateContainer>>;
          if constexpr (!empty_v<applicable_state_containers_list_type>) {
            if (entry_coodinator.has_scheduled_state(container)) {
              container.on_entry(entry_coodinator, machine, event);
            } else {
              container.on_initial_entry(machine);
            }
          } else {
            container.on_initial_entry(machine);
          }
        };
    if (parent_state_container_.is_inactive()) {
      enter_container(entry_coordinator, machine, event,
                      parent_state_container_);
    }
    enter_container(entry_coordinator, machine, event, child_state_container_);
  }

  template <typename TransitionCoordinator, concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr bool
  on_event(TransitionCoordinator &incoming_transition_coordinator,
           Machine &machine, Event const &event) {
    child_transition_coordinator ctc{incoming_transition_coordinator,
                                     parent_state_container_,
                                     child_state_container_};
    if (!child_state_container_.on_event(ctc, machine, event)) {
      node_transition_coordinator parent_transition_coordinator{
          incoming_transition_coordinator, *this};
      bool const result = parent_state_container_.on_event(
          parent_transition_coordinator, machine, event);
      if (parent_transition_coordinator.will_exit_container()) {
        child_state_container_.on_final_exit(machine);
        // The parent state container has already been deactivated during the
        // call to on_event, but haven't exited yet. We just need to invoke the
        // exit callback on the state.  This ensures that we exit the parent
        // state after the child state.
        exit(parent_state_container_.state_, machine, event);
      }
      return result;
    } else if (ctc.child_to_parent_transition_triggered()) {
      reenter(parent_state_container_.state_, machine, event);
      child_state_container_.on_initial_entry(machine);
      return true;
    } else if (ctc.will_exit_container()) {
      parent_state_container_.on_final_exit(machine);
      return true;
    } else {
      return true;
    }
  }

private:
  parent_state_container<ParentState> parent_state_container_;
  ChildStateContainer child_state_container_;
};
} // namespace hierarchical_state_container_details_

template <concepts::state ParentState,
          concepts::state_container ChildStateContainer>
using hierarchical_states =
    hierarchical_state_container_details_::container<ParentState,
                                                     ChildStateContainer>;

} // namespace skizzay::fsm