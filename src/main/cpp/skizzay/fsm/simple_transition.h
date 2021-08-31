#pragma once

#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {

namespace simple_transition_details_ {
template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event>
struct impl {
  using current_state_type = CurrentState;
  using next_state_type = NextState;
  using event_type = Event;

  template <concepts::machine_for<
      impl<current_state_type, next_state_type, event_type>>
                Machine>
  constexpr bool accepts(current_state_type const &, Machine const &,
                         event_type const &) const noexcept {
    return true;
  }
};
} // namespace simple_transition_details_

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event>
using simple_transition =
    simple_transition_details_::impl<CurrentState, NextState, Event>;
} // namespace skizzay::fsm