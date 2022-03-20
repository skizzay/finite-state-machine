#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/transition.h"

namespace skizzay::fsm {
namespace accepts_details_ {
template <typename... Ts> void accepts(Ts const &...) = delete;

struct accepts_fn final {
  constexpr bool
  operator()(concepts::state auto const &,
             concepts::event_context auto const &) const noexcept {
    return true;
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State const &s, EventContext const &e) {
    { s.accepts(e) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(State const &state,
                            EventContext const &event_context) const noexcept {
    return state.accepts(event_context);
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State const &s, EventContext const &e) {
    { accepts(s, e) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(State const &state,
                            EventContext const &event_context) const noexcept {
    return accepts(state, event_context);
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State const &s, EventContext const &e) {
    { s.accepts(e.event()) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(State const &state,
                            EventContext const &event_context) const noexcept {
    return state.accepts(event_context.event());
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State const &s, EventContext const &e) {
    { accepts(s, e.event()) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(State const &state,
                            EventContext const &event_context) const noexcept {
    return accepts(state, event_context.event());
  }
};
} // namespace accepts_details_

inline namespace accepts_fn_ {
inline constexpr accepts_details_::accepts_fn accepts = {};
} // namespace accepts_fn_

} // namespace skizzay::fsm
