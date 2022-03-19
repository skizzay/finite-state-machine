#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

namespace skizzay::fsm {

namespace reenter_fn_details_ {
template <typename... Ts> void on_reentry(Ts const &...) = delete;

struct reenter_fn final {
  constexpr void operator()(concepts::state auto &,
                            concepts::event_context auto &) const noexcept {}

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    state.on_reentry(event_context.event());
  }
  constexpr void operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(state.on_reentry(event_context.event()))) {
    state.on_reentry(event_context.event());
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    on_reentry(state, event_context.event());
  }
  constexpr void operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(on_reentry(state, event_context.event()))) {
    on_reentry(state, event_context.event());
  }
};
} // namespace reenter_fn_details_

inline namespace reenter_fn_ns {
inline constexpr reenter_fn_details_::reenter_fn reenter = {};
}

} // namespace skizzay::fsm