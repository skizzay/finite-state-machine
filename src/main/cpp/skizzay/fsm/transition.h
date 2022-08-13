#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/states_list.h"

#include <concepts>
#include <functional>
#include <type_traits>

namespace skizzay::fsm {
namespace concepts {
template <typename T>
concept transition = requires {
  typename event_t<T>;
  typename current_state_t<T>;
  typename next_state_t<T>;
};

template <typename T>
concept self_transition =
    transition<T> && std::same_as<current_state_t<std::remove_cvref_t<T>>,
                                  next_state_t<std::remove_cvref_t<T>>>;
} // namespace concepts

template <concepts::event Event, concepts::transition Transition>
struct is_event_in<Event, Transition>
    : std::is_convertible<add_cref_t<Event>, add_cref_t<event_t<Transition>>> {
};

template <typename T>
using is_transition = std::bool_constant<concepts::transition<T>>;

template <typename T>
using is_self_transition = std::bool_constant<concepts::self_transition<T>>;

namespace transition_details_ {
template <concepts::state... States> struct fake_state_provider final {
  using states_list_type = states_list<States...>;

  template <concepts::state_in<states_list_type> State> State &state() noexcept;

  template <concepts::state_in<states_list_type> State>
  State const &state() const noexcept;
};

template <typename Guard, typename CurrentState, typename Event>
concept guard_for = concepts::state<CurrentState> && concepts::event<Event> &&
    ((std::predicate<add_cref_t<Guard>, add_cref_t<CurrentState>,
                     add_cref_t<Event>> &&
      std::is_nothrow_invocable_v<add_cref_t<Guard>, add_cref_t<CurrentState>,
                                  add_cref_t<Event>>) ||
     (std::predicate<add_cref_t<Guard>, add_cref_t<CurrentState>,
                     add_cref_t<Event>,
                     add_cref_t<fake_state_provider<CurrentState>>> &&
      std::is_nothrow_invocable_v<
          add_cref_t<Guard>, add_cref_t<CurrentState>, add_cref_t<Event>,
          add_cref_t<fake_state_provider<CurrentState>>>));

template <typename Trigger, typename Event>
concept trigger_for = concepts::event<Event> &&
    (std::invocable<Trigger, add_cref_t<Event>> || std::invocable<Trigger>);

template <typename... Ts> bool is_accepted(Ts const &...) = delete;

struct defer_to_state_acceptance {
  template <concepts::state CurrentState, concepts::event Event>
  requires requires(CurrentState const &current_state, Event const &event) {
    { current_state.is_accepted(event) }
    noexcept->concepts::boolean;
  }
  constexpr concepts::boolean auto
  operator()(CurrentState const &current_state,
             Event const &event) const noexcept {
    return current_state.is_accepted(event);
  }

  template <concepts::state CurrentState, concepts::event Event>
  requires requires(CurrentState const &current_state, Event const &event) {
    { is_accepted(current_state, event) }
    noexcept->concepts::boolean;
  }
  constexpr concepts::boolean auto
  operator()(CurrentState const &current_state,
             Event const &event) const noexcept {
    return is_accepted(current_state, event);
  }

  template <concepts::state CurrentState, concepts::event Event,
            concepts::state_provider StateProvider>
  requires requires(add_cref_t<CurrentState> current_state,
                    add_cref_t<Event> event,
                    StateProvider const &state_provider) {
    { current_state.is_accepted(event, state_provider) }
    noexcept->concepts::boolean;
  }
  constexpr concepts::boolean auto
  operator()(CurrentState const &current_state, Event const &event,
             StateProvider const &state_provider) const noexcept {
    return current_state.is_accepted(event, state_provider);
  }

  template <concepts::state CurrentState, concepts::event Event,
            concepts::state_provider StateProvider>
  requires requires(add_cref_t<CurrentState> current_state,
                    add_cref_t<Event> event,
                    StateProvider const &state_provider) {
    { is_accepted(current_state, event, state_provider) }
    noexcept->concepts::boolean;
  }
  constexpr concepts::boolean auto
  operator()(CurrentState const &current_state, Event const &event,
             StateProvider const &state_provider) const noexcept {
    return is_accepted(current_state, event, state_provider);
  }

  constexpr bool operator()(concepts::state auto const &,
                            concepts::event auto const &) const noexcept {
    return true;
  }
};

template <typename... Ts> void on_triggered(Ts const &...) = delete;

struct defer_to_event_trigger {
  template <concepts::event Event>
  requires requires(Event const &event) { event.on_triggered(); }
  constexpr void operator()(Event const &event) const
      noexcept(noexcept(event.on_triggered())) {
    event.on_triggered();
  }
  template <concepts::event Event>
  requires requires(Event const &event) { on_triggered(event); }
  constexpr void operator()(Event const &event) const
      noexcept(noexcept(on_triggered(event))) {
    on_triggered(event);
  }

  constexpr void operator()(concepts::event auto const &) const noexcept {}
};
} // namespace transition_details_

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event,
          transition_details_::guard_for<CurrentState, Event> Guard,
          transition_details_::trigger_for<Event> OnTriggered>
struct basic_transition {
  using current_state_type = std::remove_cvref_t<CurrentState>;
  using next_state_type = std::remove_cvref_t<NextState>;
  using event_type = std::remove_cvref_t<Event>;

  constexpr basic_transition() noexcept(
      std::is_nothrow_default_constructible_v<Guard>
          &&std::is_nothrow_default_constructible_v<OnTriggered>) requires
      std::is_default_constructible_v<Guard> &&
      std::is_default_constructible_v<OnTriggered>
  = default;

  constexpr explicit basic_transition(Guard guard) noexcept(
      std::is_nothrow_move_constructible_v<Guard>
          &&std::is_nothrow_default_constructible_v<OnTriggered>) requires
      std::is_default_constructible_v<OnTriggered>
      : guard_{std::move_if_noexcept(guard)} {}

  constexpr explicit basic_transition(OnTriggered on_triggered) noexcept(
      std::is_nothrow_default_constructible_v<Guard>
          &&std::is_nothrow_move_constructible_v<OnTriggered>) requires
      std::is_default_constructible_v<Guard>
      : on_triggered_{std::move_if_noexcept(on_triggered)} {}

  constexpr explicit basic_transition(
      Guard guard,
      OnTriggered
          on_triggered) noexcept(std::is_nothrow_move_constructible_v<Guard>
                                     &&std::is_nothrow_move_constructible_v<
                                         OnTriggered>)
      : guard_{std::move_if_noexcept(guard)},
        on_triggered_{std::move_if_noexcept(on_triggered)} {}

  constexpr concepts::boolean auto
  is_accepted(current_state_type const &current_state,
              event_type const &event) const noexcept requires
      std::predicate<add_cref_t<Guard>, add_cref_t<current_state_type>,
                     add_cref_t<event_type>> {
    return std::invoke(this->guard_, current_state, event);
  }

  template <concepts::state_provider StateProvider>
  requires std::predicate<add_cref_t<Guard>, add_cref_t<current_state_type>,
                          add_cref_t<event_type>, add_cref_t<StateProvider>>
  constexpr concepts::boolean auto
  is_accepted(current_state_type const &current_state, event_type const &event,
              StateProvider const &state_provider) const
      noexcept(std::is_nothrow_invocable_v<
               add_cref_t<Guard>, add_cref_t<current_state_type>,
               add_cref_t<event_type>, add_cref_t<StateProvider>>) {
    return std::invoke(this->guard_, current_state, event, state_provider);
  }

  constexpr void on_triggered(Event const &event) noexcept(
      std::is_nothrow_invocable_v<OnTriggered, Event const &>) requires
      std::invocable<OnTriggered, Event const &> {
    std::invoke(this->on_triggered_, event);
  }

  constexpr void
  on_triggered() noexcept(std::is_nothrow_invocable_v<OnTriggered>) requires
      std::invocable<OnTriggered> {
    std::invoke(this->on_triggered_);
  }

private : [[no_unique_address]] Guard guard_;
  [[no_unique_address]] OnTriggered on_triggered_;
};

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event>
using simple_transition =
    basic_transition<CurrentState, NextState, Event,
                     transition_details_::defer_to_state_acceptance,
                     transition_details_::defer_to_event_trigger>;

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event,
          transition_details_::guard_for<CurrentState, Event> Guard>
using guarded_transition =
    basic_transition<CurrentState, NextState, Event, Guard,
                     transition_details_::defer_to_event_trigger>;

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event,
          transition_details_::trigger_for<Event> OnTriggered>
using action_transition =
    basic_transition<CurrentState, NextState, Event,
                     transition_details_::defer_to_state_acceptance,
                     OnTriggered>;

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event,
          transition_details_::guard_for<CurrentState, Event> Guard,
          transition_details_::trigger_for<Event> OnTriggered>
using guarded_action_transition =
    basic_transition<CurrentState, NextState, Event, Guard, OnTriggered>;

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event,
          transition_details_::guard_for<CurrentState, Event> Guard>
constexpr guarded_transition<CurrentState, NextState, Event,
                             std::remove_reference_t<Guard>>
transition_guarded_by(Guard &&guard) noexcept(
    std::is_nothrow_constructible_v<
        guarded_transition<CurrentState, NextState, Event,
                           std::remove_reference_t<Guard>>,
        Guard>) {
  return guarded_transition<CurrentState, NextState, Event,
                            std::remove_reference_t<Guard>>{
      std::forward<Guard>(guard)};
}

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, transition_details_::trigger_for<Event> Action>
constexpr action_transition<CurrentState, NextState, Event,
                            std::remove_reference_t<Action>>
action_transition_for(Action &&action) noexcept(
    std::is_nothrow_constructible_v<
        action_transition<CurrentState, NextState, Event,
                          std::remove_reference_t<Action>>,
        Action>) {
  return action_transition<CurrentState, NextState, Event,
                           std::remove_reference_t<Action>>{
      std::forward<Action>(action)};
}

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event,
          transition_details_::guard_for<CurrentState, Event> Guard,
          transition_details_::trigger_for<Event> Action>
constexpr guarded_action_transition<CurrentState, NextState, Event,
                                    std::remove_reference_t<Guard>,
                                    std::remove_reference_t<Action>>
transition_guarded_by_with_action(Guard &&guard, Action &&action) noexcept(
    std::is_nothrow_constructible_v<
        guarded_action_transition<CurrentState, NextState, Event,
                                  std::remove_reference_t<Guard>,
                                  std::remove_reference_t<Action>>,
        Guard, Action>) {
  return guarded_action_transition<CurrentState, NextState, Event,
                                   std::remove_reference_t<Guard>,
                                   std::remove_reference_t<Action>>{
      std::forward<Guard>(guard), std::forward<Action>(action)};
}
} // namespace skizzay::fsm