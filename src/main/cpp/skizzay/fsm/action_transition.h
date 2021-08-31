#pragma once

#include <concepts>
#include <functional>
#include <skizzay/fsm/concepts.h>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace action_transition_details_ {
template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, std::invocable<Event const &> Action>
struct impl {
  using current_state_type = CurrentState;
  using next_state_type = NextState;
  using event_type = Event;

  constexpr explicit impl(Action action) noexcept(
      std::is_nothrow_move_constructible_v<Action>)
      : action_{std::move_if_noexcept(action)} {}

  template <concepts::machine_for<
      impl<current_state_type, next_state_type, event_type, Action>>
                Machine>
  constexpr bool accepts(current_state_type const &current_state,
                         Machine const &machine,
                         event_type const &event) const noexcept {
    return true;
  }

  constexpr void on_triggered(Event const &event) noexcept(
      std::is_nothrow_invocable_v<Action, Event const &>) {
    std::invoke(action_, event);
  }

private:
  [[no_unique_address]] Action action_;
};
} // namespace action_transition_details_

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, std::invocable<Event const &> Action>
using action_transition =
    action_transition_details_::impl<CurrentState, NextState, Event, Action>;
} // namespace skizzay::fsm