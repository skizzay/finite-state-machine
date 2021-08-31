#pragma once

#include <concepts>
#include <functional>
#include <skizzay/fsm/concepts.h>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace guarded_transition_details_ {
template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, typename Guard>
struct impl {
  using current_state_type = CurrentState;
  using next_state_type = NextState;
  using event_type = Event;

  constexpr explicit impl(Guard guard) noexcept(
      std::is_nothrow_move_constructible_v<Guard>)
      : guard_{std::move_if_noexcept(guard)} {}

  template <concepts::machine_for<
      impl<current_state_type, next_state_type, event_type, Guard>>
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
      impl<current_state_type, next_state_type, event_type, Guard>>
                Machine>
  requires std::is_invocable_r_v<bool, Guard const &,
                                 current_state_type const &, event_type const &>
  constexpr bool accepts(current_state_type const &current_state,
                         Machine const &,
                         event_type const &event) const noexcept {
    return std::invoke(guard_, current_state, event);
  }

private:
  [[no_unique_address]] Guard guard_;
};
} // namespace guarded_transition_details_

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, typename Guard>
using guarded_transition =
    guarded_transition_details_::impl<CurrentState, NextState, Event, Guard>;
} // namespace skizzay::fsm