#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

#include <utility>

namespace skizzay::fsm {

namespace enter_fn_details_ {
template <typename... Ts> void on_entry(Ts const &...) = delete;

template <typename State, typename EventContext>
concept event_enterable = requires(State &state, EventContext &event_context) {
  state.on_entry(event_context.event());
}
|| requires(State &state, EventContext &event_context) {
  on_entry(state, event_context.event());
};

template <typename State, typename EventContext>
concept event_context_enterable = requires(State &state,
                                           EventContext &event_context) {
  state.on_entry(event_context);
}
|| requires(State &state, EventContext &event_context) {
  on_entry(state, event_context);
};

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

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    state.on_entry(event_context);
  } &&(!event_enterable<State, EventContext>)constexpr void
  operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(state.on_entry(event_context))) {
    state.on_entry(event_context);
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state, EventContext &event_context) {
    on_entry(state, event_context);
  } &&(!event_enterable<State, EventContext>)constexpr void
  operator()(State &state, EventContext &event_context) const
      noexcept(noexcept(on_entry(state, event_context))) {
    on_entry(state, event_context);
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state) {
    state.on_entry();
  } &&(!event_enterable<State, EventContext> &&
       !event_context_enterable<State, EventContext>)constexpr void
  operator()(State &state, EventContext &) const
      noexcept(noexcept(state.on_entry())) {
    state.on_entry();
  }

  template <concepts::state State, concepts::event_context EventContext>
  requires requires(State &state) {
    on_entry(state);
  } &&(!event_enterable<State, EventContext> &&
       !event_context_enterable<State, EventContext>)constexpr void
  operator()(State &state, EventContext &) const
      noexcept(noexcept(on_entry(state))) {
    on_entry(state);
  }
};
} // namespace enter_fn_details_

inline namespace enter_fn_ns {
inline constexpr enter_fn_details_::enter_fn enter = {};
}

} // namespace skizzay::fsm