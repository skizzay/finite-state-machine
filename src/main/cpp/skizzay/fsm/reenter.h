#pragma once

#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {

namespace reenter_fn_details_ {
template <typename State, typename Machine, typename Event>
void on_reentry(State &, Machine &, Event const &) = delete;
template <typename State, typename Event>
void on_reentry(State &, Event const &) = delete;
template <typename State, typename Machine>
void on_reentry(State &, Machine &) = delete;
template <typename State> void on_reentry(State &) = delete;

struct reenter_fn final {
  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine, Event const &event) {
    state.on_reentry(machine, event);
  }
  constexpr void operator()(State &state, Machine &machine,
                            Event const &event) const
      noexcept(noexcept(state.on_reentry(machine, event))) {
    state.on_reentry(machine, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine, Event const &event) {
    on_reentry(state, machine, event);
  }
  constexpr void operator()(State &state, Machine &machine,
                            Event const &event) const
      noexcept(noexcept(on_reentry(state, machine, event))) {
    on_reentry(state, machine, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Event const &event) {
    state.on_reentry(event);
  }
  constexpr void operator()(State &state, Machine &, Event const &event) const
      noexcept(noexcept(state.on_reentry(event))) {
    state.on_reentry(event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Event const &event) {
    on_reentry(state, event);
  }
  constexpr void operator()(State &state, Machine &, Event const &event) const
      noexcept(noexcept(on_reentry(state, event))) {
    on_reentry(state, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine) {
    state.on_reentry(machine);
  }
  constexpr void operator()(State &state, Machine &machine, Event const &) const
      noexcept(noexcept(state.on_reentry(machine))) {
    state.on_reentry(machine);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine) {
    on_reentry(state, machine);
  }
  constexpr void operator()(State &state, Machine &machine, Event const &) const
      noexcept(noexcept(on_reentry(state, machine))) {
    on_reentry(state, machine);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state) { state.on_reentry(); }
  constexpr void operator()(State &state, Machine &, Event const &) const
      noexcept(noexcept(state.on_reentry())) {
    state.on_reentry();
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state) { on_reentry(state); }
  constexpr void operator()(State &state, Machine &, Event const &) const
      noexcept(noexcept(on_reentry(state))) {
    on_reentry(state);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  constexpr void operator()(State &, Machine &, Event const &) const noexcept {}
};
} // namespace reenter_fn_details_

inline namespace reenter_fn_ns {
inline constexpr reenter_fn_details_::reenter_fn reenter = {};
}

} // namespace skizzay::fsm