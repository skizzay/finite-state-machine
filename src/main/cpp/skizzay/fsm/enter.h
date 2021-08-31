#pragma once

#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {

namespace enter_fn_details_ {
template <typename State, typename Machine, typename Event>
void on_entry(State &, Machine &, Event const &) = delete;
template <typename State, typename Event>
void on_entry(State &, Event const &) = delete;
template <typename State, typename Machine>
void on_entry(State &, Machine &) = delete;
template <typename State> void on_entry(State &) = delete;

struct enter_fn final {
  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine, Event const &event) {
    state.on_entry(machine, event);
  }
  constexpr void operator()(State &state, Machine &machine,
                            Event const &event) const
      noexcept(noexcept(state.on_entry(machine, event))) {
    state.on_entry(machine, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine, Event const &event) {
    on_entry(state, machine, event);
  }
  constexpr void operator()(State &state, Machine &machine,
                            Event const &event) const
      noexcept(noexcept(on_entry(state, machine, event))) {
    on_entry(state, machine, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Event const &event) { state.on_entry(event); }
  constexpr void operator()(State &state, Machine &, Event const &event) const
      noexcept(noexcept(state.on_entry(event))) {
    state.on_entry(event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Event const &event) {
    on_entry(state, event);
  }
  constexpr void operator()(State &state, Machine &, Event const &event) const
      noexcept(noexcept(on_entry(state, event))) {
    on_entry(state, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine) { state.on_entry(machine); }
  constexpr void operator()(State &state, Machine &machine, Event const &) const
      noexcept(noexcept(state.on_entry(machine))) {
    state.on_entry(machine);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine) {
    on_entry(state, machine);
  }
  constexpr void operator()(State &state, Machine &machine, Event const &) const
      noexcept(noexcept(on_entry(state, machine))) {
    on_entry(state, machine);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state) { state.on_entry(); }
  constexpr void operator()(State &state, Machine &, Event const &) const
      noexcept(noexcept(state.on_entry())) {
    state.on_entry();
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state) { on_entry(state); }
  constexpr void operator()(State &state, Machine &, Event const &) const
      noexcept(noexcept(on_entry(state))) {
    on_entry(state);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  constexpr void operator()(State &, Machine &, Event const &) const noexcept {}
};
} // namespace enter_fn_details_

inline namespace enter_fn_ns {
inline constexpr enter_fn_details_::enter_fn enter = {};
}

} // namespace skizzay::fsm