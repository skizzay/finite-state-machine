#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_provider.h"

namespace skizzay::fsm {

namespace reenter_fn_details_ {
template <typename... Ts> void on_reentry(Ts const &...) = delete;

struct reenter_fn final {
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
    s.on_reentry(e, ee, sp);
  }
  constexpr void operator()(State &state, Event const &event,
                            EventEngine &event_engine,
                            StateProvider &state_provider) const {
    state.on_reentry(event, event_engine, state_provider);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e) { s.on_reentry(e); }
  constexpr void operator()(State &state, Event const &event, EventEngine &,
                            StateProvider &) const {
    state.on_reentry(event);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s) { s.on_reentry(); }
  constexpr void operator()(State &state, Event const &, EventEngine &,
                            StateProvider &) const {
    state.on_reentry();
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e, EventEngine &ee,
                    StateProvider &sp) {
    on_reentry(s, e, ee, sp);
  }
  constexpr void operator()(State &state, Event const &event,
                            EventEngine &event_engine,
                            StateProvider &state_provider) const {
    on_reentry(state, event, event_engine, state_provider);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e) { on_reentry(s, e); }
  constexpr void operator()(State &state, Event const &event, EventEngine &,
                            StateProvider &) const {
    on_reentry(state, event);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s) { on_reentry(s); }
  constexpr void operator()(State &state, Event const &, EventEngine &,
                            StateProvider &) const {
    on_reentry(state);
  }
};
} // namespace reenter_fn_details_

inline namespace reenter_fn_ns {
inline constexpr reenter_fn_details_::reenter_fn reenter = {};
}

} // namespace skizzay::fsm