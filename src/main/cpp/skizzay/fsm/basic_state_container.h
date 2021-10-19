#pragma once

#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/container_status.h>
#include <skizzay/fsm/enter.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/exit.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/reenter.h>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/trigger.h>
#include <skizzay/fsm/type_list.h>
#include <type_traits>

namespace skizzay::fsm {

namespace basic_state_container_details_ {

enum class acceptance_type { unaccepted, reentered, exited };

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

  constexpr bool is_active() const noexcept { return active_; }

  constexpr bool is_inactive() const noexcept { return !active_; }

  template <concepts::state S> constexpr bool is() const noexcept {
    return std::is_same_v<State, S> && is_active();
  }

  template <std::same_as<State>>
  constexpr optional_reference<State const> current_state() const noexcept {
    return current_state_;
  }

  template <std::same_as<State>> constexpr State const &get() const noexcept {
    return current_state_;
  }

  template <std::same_as<State>> constexpr State &get() noexcept {
    return current_state_;
  }

  template <std::same_as<State>, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return ancestry.with_new_generation(current_state_);
  }

  template <concepts::machine_for<State> Machine>
  constexpr void on_initial_entry(Machine &machine) {
    do_entry(machine, initial_entry_event);
  }

  template <concepts::entry_coordinator EntryCoordinator,
            concepts::machine_for<State> Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr void on_entry(EntryCoordinator const &entry_coordinator,
                          Machine &machine, Event const &event) {
    if (entry_coordinator.template is_scheduled<State>()) {
      do_entry(machine, event);
    }
  }

  template <concepts::machine_for<State> Machine>
  constexpr void on_final_exit(Machine &machine) {
    do_exit(machine, final_exit_event);
  }

  template <typename TransitionCoordinator, concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr bool on_event(TransitionCoordinator &transition_coordinator,
                          Machine &machine, Event const &event) {
    switch (attempt_transitions(transition_coordinator, machine, event)) {
    case acceptance_type::reentered:
      reenter(current_state_, machine, event);
      return true;

    case acceptance_type::exited:
      do_exit(machine, event);
      return true;

    default:
      return false;
    }
  }

private:
  bool active_ = false;

  template <typename Machine, typename Event>
  constexpr void do_entry(Machine &machine, Event const &event) {
    assert(is_inactive() && "Entry made on active state");
    enter(current_state_, machine, event);
    active_ = true;
  }

  template <typename Machine, typename Event>
  constexpr void do_exit(Machine &machine, Event const &event) {
    assert(is_active() && "Exiting inactive state");
    active_ = false;
    exit(current_state_, machine, event);
  }

  template <typename TransitionCoordinator, typename Machine, typename Event>
  constexpr acceptance_type
  attempt_transitions(TransitionCoordinator &transition_coordinator,
                      Machine &machine, Event const &event) {
    acceptance_type acceptance = acceptance_type::unaccepted;
    auto attempt_to_transition = [&]<concepts::transition Transition>(
                                     Transition &transition) {
      if (transition.accepts(std::as_const(current_state_),
                             std::as_const(machine), event)) {
        if constexpr (concepts::self_transition<Transition>) {
          assert(acceptance_type::exited != acceptance &&
                 "Self transition encountered but state has already exited");
          skizzay::fsm::trigger(transition, event);
          acceptance = acceptance_type::reentered;
        } else {
          assert(acceptance_type::reentered != acceptance &&
                 "State changing transition encountered but state has "
                 "already reentered");
          transition_coordinator.on_transition(transition, machine, event);
          acceptance = acceptance_type::exited;
        }
      }
    };
    std::apply(
        [attempt_to_transition](concepts::transition auto &...transitions) {
          (attempt_to_transition(transitions), ...);
        },
        transition_coordinator.get_transitions(std::as_const(current_state_)));
    return acceptance;
  }
};
} // namespace basic_state_container_details_

template <typename State>
using basic_state_container = basic_state_container_details_::container<State>;
} // namespace skizzay::fsm