#pragma once

#include <skizzay/fsm/ancestors.h>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/event.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/reenter.h>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/trigger.h>
#include <skizzay/fsm/type_list.h>
#include <type_traits>

namespace skizzay::fsm {

namespace basic_state_container_details_ {

enum class acceptance_type { unaccepted, reentered, exited };

template <typename State, typename DerivedStateContainer>
concept derived_container_state = concepts::state<State> &&
    requires(DerivedStateContainer &dsc, DerivedStateContainer const &dscc) {
  { dsc.get() } -> std::same_as<State &>;
  { dscc.get() } -> std::same_as<State const &>;
};

template <typename Derived>
using state_t = std::remove_cvref_t<decltype(std::declval<Derived>().get())>;

template <typename Derived> struct container {
  friend Derived;

  constexpr bool is_active() const noexcept { return active_; }

  constexpr bool is_inactive() const noexcept { return !active_; }

  template <derived_container_state<Derived>>
  constexpr bool is() const noexcept {
    return is_active();
  }

  template <derived_container_state<Derived>>
  constexpr auto current_state() const noexcept {
    using result_type = optional_reference<state_t<Derived> const>;
    return is_active() ? result_type{derived().get()}
                       : result_type{std::nullopt};
  }

  template <derived_container_state<Derived>, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return ancestry.with_new_generation(derived().get());
  }

  template <concepts::machine Machine>
  constexpr void on_initial_entry(Machine &machine) {
    activate(machine, initial_entry_event);
  }

  template <concepts::entry_coordinator EntryCoordinator,
            concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr void on_entry(EntryCoordinator const &entry_coordinator,
                          Machine &machine, Event const &event) {
    if (entry_coordinator.template is_scheduled<state_t<Derived>>()) {
      activate(machine, event);
    }
  }

  template <concepts::machine Machine>
  constexpr void on_final_exit(Machine &machine) {
    deactivate(machine, final_exit_event);
  }

  template <typename TransitionCoordinator, concepts::machine Machine,
            concepts::event_in<events_list_t<Machine>> Event>
  constexpr bool on_event(TransitionCoordinator &transition_coordinator,
                          Machine &machine, Event const &event) {
    switch (attempt_transitions(transition_coordinator, machine, event)) {
    case acceptance_type::reentered:
      reenter(derived().get(), machine, event);
      return true;

    case acceptance_type::exited:
      deactivate(machine, event);
      return true;

    default:
      return false;
    }
  }

private:
  bool active_ = false;

  constexpr Derived &derived() noexcept {
    return *static_cast<Derived *>(this);
  }

  constexpr Derived const &derived() const noexcept {
    return *static_cast<Derived const *>(this);
  }

  template <typename Machine, typename Event>
  constexpr void activate(Machine &machine, Event const &event) {
    assert(is_inactive() && "Entry made on active state");
    derived().do_entry(machine, event);
    active_ = true;
  }

  template <typename Machine, typename Event>
  constexpr void deactivate(Machine &machine, Event const &event) {
    assert(is_active() && "Exiting inactive state");
    active_ = false;
    derived().do_exit(machine, event);
  }

  template <typename TransitionCoordinator, typename Machine, typename Event>
  constexpr acceptance_type
  attempt_transitions(TransitionCoordinator &transition_coordinator,
                      Machine &machine, Event const &event) {
    auto const &const_self = *const_cast<container<Derived> const *>(this);
    acceptance_type acceptance = acceptance_type::unaccepted;
    auto attempt_to_transition = [&]<concepts::transition Transition>(
                                     Transition &transition) {
      if (transition.accepts(const_self.derived().get(), std::as_const(machine),
                             event)) {
        if constexpr (concepts::self_transition<Transition>) {
          assert(acceptance_type::exited != acceptance &&
                 "Self transition encountered but state has already exited");
          trigger(transition, event);
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
        transition_coordinator.get_transitions(const_self.derived().get()));
    return acceptance;
  }
};
} // namespace basic_state_container_details_

template <typename Derived>
using basic_state_container =
    basic_state_container_details_::container<Derived>;
} // namespace skizzay::fsm