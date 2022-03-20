#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

#include <type_traits>

namespace skizzay::fsm {

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event>
struct simple_transition {
  using current_state_type = std::remove_cvref_t<CurrentState>;
  using next_state_type = std::remove_cvref_t<NextState>;
  using event_type = std::remove_cvref_t<Event>;

  template <concepts::event_context_for<event_type> EventContext>
  requires requires(current_state_type const &s, EventContext const &e) {
    { s.accepts(e.event()) }
    noexcept->concepts::boolean;
  }
  constexpr bool accepts(current_state_type const &state,
                         EventContext const &event_context) const noexcept {
    return state.accepts(event_context.event());
  }

  template <concepts::event_context_for<event_type> EventContext>
  requires requires(current_state_type const &s, EventContext const &e) {
    { s.accepts(e) }
    noexcept->concepts::boolean;
  }
  constexpr bool accepts(current_state_type const &state,
                         EventContext const &event_context) const noexcept {
    return state.accepts(event_context);
  }
};
} // namespace skizzay::fsm