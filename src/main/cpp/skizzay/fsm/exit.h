#pragma once

#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {

namespace exit_fn_details_ {
template <typename State, typename Machine, typename Event>
void on_exit(State &, Machine &, Event const &) = delete;
template <typename State, typename Event>
void on_exit(State &, Event const &) = delete;
template <typename State, typename Machine>
void on_exit(State &, Machine &) = delete;
template <typename State> void on_exit(State &) = delete;

struct exit_fn final {
  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine, Event const &event) {
    state.on_exit(machine, event);
  }
  constexpr void operator()(State &state, Machine &machine,
                            Event const &event) const
      noexcept(noexcept(state.on_exit(machine, event))) {
    state.on_exit(machine, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine, Event const &event) {
    on_exit(state, machine, event);
  }
  constexpr void operator()(State &state, Machine &machine,
                            Event const &event) const
      noexcept(noexcept(on_exit(state, machine, event))) {
    on_exit(state, machine, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Event const &event) { state.on_exit(event); }
  constexpr void operator()(State &state, Machine &, Event const &event) const
      noexcept(noexcept(state.on_exit(event))) {
    state.on_exit(event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Event const &event) { on_exit(state, event); }
  constexpr void operator()(State &state, Machine &, Event const &event) const
      noexcept(noexcept(on_exit(state, event))) {
    on_exit(state, event);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine) { state.on_exit(machine); }
  constexpr void operator()(State &state, Machine &machine, Event const &) const
      noexcept(noexcept(state.on_exit(machine))) {
    state.on_exit(machine);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state, Machine &machine) { on_exit(state, machine); }
  constexpr void operator()(State &state, Machine &machine, Event const &) const
      noexcept(noexcept(on_exit(state, machine))) {
    on_exit(state, machine);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state) { state.on_exit(); }
  constexpr void operator()(State &state, Machine &, Event const &) const
      noexcept(noexcept(state.on_exit())) {
    state.on_exit();
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  requires requires(State &state) { on_exit(state); }
  constexpr void operator()(State &state, Machine &, Event const &) const
      noexcept(noexcept(on_exit(state))) {
    on_exit(state);
  }

  template <concepts::state State, concepts::machine Machine,
            concepts::event Event>
  constexpr void operator()(State &, Machine &, Event const &) const noexcept {}
};
} // namespace exit_fn_details_

inline namespace exit_fn_ns {
inline constexpr exit_fn_details_::exit_fn exit = {};
}

} // namespace skizzay::fsm