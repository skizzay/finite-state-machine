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

  template <concepts::transition Transition, concepts::state State, concepts::event_context EventContext>
  requires requires(Transition const &t, State const &s, EventContext const &e) {
    { t.accepts(s, e) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(Transition const &transition, State const &state,
                            EventContext const &event_context) const noexcept {
    return transition.accepts(state, event_context);
  }
};
} // namespace accepts_details_

inline namespace accepts_fn_ {
inline constexpr accepts_details_::accepts_fn accepts = {};
} // namespace accepts_fn_

} // namespace skizzay::fsm
