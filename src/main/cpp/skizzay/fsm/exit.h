#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_provider.h"

namespace skizzay::fsm {

namespace exit_fn_details_ {
template <typename... Ts> void on_exit(Ts const &...) = delete;

struct exit_fn final {
  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  constexpr void operator()(State &, Event const &, EventEngine &,
                            StateProvider &) const noexcept {}

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e, EventEngine &ee,
                    StateProvider &sp) {
    s.on_exit(e, ee, sp);
  }
  constexpr void operator()(State &state, Event const &event,
                            EventEngine &event_engine,
                            StateProvider &state_provider) const {
    state.on_exit(event, event_engine, state_provider);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e) { s.on_exit(e); }
  constexpr void operator()(State &state, Event const &event, EventEngine &,
                            StateProvider &) const {
    state.on_exit(event);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s) { s.on_exit(); }
  constexpr void operator()(State &state, Event const &, EventEngine &,
                            StateProvider &) const {
    state.on_exit();
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e, EventEngine &ee,
                    StateProvider &sp) {
    on_exit(s, e, ee, sp);
  }
  constexpr void operator()(State &state, Event const &event,
                            EventEngine &event_engine,
                            StateProvider &state_provider) const {
    on_exit(state, event, event_engine, state_provider);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e) { on_exit(s, e); }
  constexpr void operator()(State &state, Event const &event, EventEngine &,
                            StateProvider &) const {
    on_exit(state, event);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s) { on_exit(s); }
  constexpr void operator()(State &state, Event const &, EventEngine &,
                            StateProvider &) const {
    on_exit(state);
  }
};
} // namespace exit_fn_details_

inline namespace exit_fn_ns {
inline constexpr exit_fn_details_::exit_fn exit = {};
}

} // namespace skizzay::fsm