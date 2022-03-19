#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

namespace skizzay::fsm {

namespace exit_fn_details_ {
template <typename... Ts> void on_exit(Ts const &...) = delete;

struct exit_fn final {
  constexpr void operator()(concepts::state auto &,
                            concepts::event_context auto &) const noexcept {}

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    state.on_exit(event_context.event());
  }
  constexpr void operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(state.on_exit(event_context.event()))) {
    state.on_exit(event_context.event());
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    on_exit(state, event_context.event());
  }
  constexpr void operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(on_exit(state, event_context.event()))) {
    on_exit(state, event_context.event());
  }
};
} // namespace exit_fn_details_

inline namespace exit_fn_ns {
inline constexpr exit_fn_details_::exit_fn exit = {};
}

} // namespace skizzay::fsm