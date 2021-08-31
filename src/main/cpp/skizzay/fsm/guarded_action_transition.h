#pragma once

#include <concepts>
#include <functional>
#include <skizzay/fsm/concepts.h>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace guarded_action_transition_details_ {
template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, typename Guard,
          std::invocable<Event const &> Action>
struct impl {
  using current_state_type = CurrentState;
  using next_state_type = NextState;
  using event_type = Event;

  constexpr explicit impl(Guard guard, Action action) noexcept(
      std::conjunction_v<std::is_nothrow_move_constructible<Guard>,
                         std::is_nothrow_move_constructible<Action>>)
      : guard_{std::move_if_noexcept(guard)}, action_{std::move_if_noexcept(
                                                  action)} {}

  template <concepts::machine_for<
      impl<current_state_type, next_state_type, event_type, Guard, Action>>
                Machine>
  requires std::is_invocable_r_v<bool, Guard const &,
                                 current_state_type const &, Machine const &,
                                 event_type const &>
  constexpr bool accepts(current_state_type const &current_state,
                         Machine const &machine,
                         event_type const &event) const noexcept {
    return std::invoke(guard_, current_state, machine, event);
  }

  template <concepts::machine_for<
      impl<current_state_type, next_state_type, event_type, Guard, Action>>
                Machine>
  requires std::is_invocable_r_v<bool, Guard const &,
                                 current_state_type const &, event_type const &>
  constexpr bool accepts(current_state_type const &current_state,
                         Machine const &,
                         event_type const &event) const noexcept {
    return std::invoke(guard_, current_state, event);
  }

  constexpr void on_triggered(Event const &event) noexcept(
      std::is_nothrow_invocable_v<Action, Event const &>) {
    std::invoke(action_, event);
  }

private:
  [[no_unique_address]] Guard guard_;
  [[no_unique_address]] Action action_;
};
} // namespace guarded_action_transition_details_

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, typename Guard,
          std::invocable<Event const &> Action>
using guarded_action_transition =
    guarded_action_transition_details_::impl<CurrentState, NextState, Event,
                                             Guard, Action>;
} // namespace skizzay::fsm