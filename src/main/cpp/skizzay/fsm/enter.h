#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

#include <utility>

namespace skizzay::fsm {

namespace enter_fn_details_ {
template <typename... Ts> void on_entry(Ts const &...) = delete;

struct enter_fn final {
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
    s.on_entry(e, ee, sp);
  }
  constexpr void operator()(State &state, Event const &event,
                            EventEngine &event_engine,
                            StateProvider &state_provider) const {
    state.on_entry(event, event_engine, state_provider);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e) { s.on_entry(e); }
  constexpr void operator()(State &state, Event const &event, EventEngine &,
                            StateProvider &) const {
    state.on_entry(event);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s) { s.on_entry(); }
  constexpr void operator()(State &state, Event const &, EventEngine &,
                            StateProvider &) const {
    state.on_entry();
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e, EventEngine &ee,
                    StateProvider &sp) {
    on_entry(s, e, ee, sp);
  }
  constexpr void operator()(State &state, Event const &event,
                            EventEngine &event_engine,
                            StateProvider &state_provider) const {
    on_entry(state, event, event_engine, state_provider);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s, Event const &e) { on_entry(s, e); }
  constexpr void operator()(State &state, Event const &event, EventEngine &,
                            StateProvider &) const {
    on_entry(state, event);
  }

  template <concepts::state State, concepts::event Event,
            concepts::event_engine EventEngine,
            concepts::state_provider StateProvider>
  requires requires(State &s) { on_entry(s); }
  constexpr void operator()(State &state, Event const &, EventEngine &,
                            StateProvider &) const {
    on_entry(state);
  }
};
} // namespace enter_fn_details_

inline namespace enter_fn_ns {
inline constexpr enter_fn_details_::enter_fn enter = {};
}

} // namespace skizzay::fsm