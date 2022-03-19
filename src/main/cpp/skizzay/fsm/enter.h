#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

#include <utility>

namespace skizzay::fsm {

namespace enter_fn_details_ {
template <typename... Ts> void on_entry(Ts const &...) = delete;

struct enter_fn final {
  constexpr void operator()(concepts::state auto &,
                            concepts::event_context auto &) const noexcept {}

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    state.on_entry(event_context.event());
  }
  constexpr void operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(state.on_entry(event_context.event()))) {
    state.on_entry(event_context.event());
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    on_entry(state, event_context.event());
  }
  constexpr void operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(on_entry(state, event_context.event()))) {
    on_entry(state, event_context.event());
  }
};
} // namespace enter_fn_details_

inline namespace enter_fn_ns {
inline constexpr enter_fn_details_::enter_fn enter = {};
}

} // namespace skizzay::fsm