#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state.h"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, typename Action>
requires std::invocable<Action, add_cref_t<Event>> || std::invocable<Action>
struct action_transition {
  using current_state_type = std::remove_cvref_t<CurrentState>;
  using next_state_type = std::remove_cvref_t<NextState>;
  using event_type = std::remove_cvref_t<Event>;

  constexpr explicit action_transition(Action action) noexcept(
      std::is_nothrow_move_constructible_v<Action>)
      : action_{std::move_if_noexcept(action)} {}

  constexpr void on_triggered(event_type const &event) noexcept(
      std::is_nothrow_invocable_v<Action, Event const &>) requires
      std::invocable<Action, add_cref_t<event_type>> {
    std::invoke(action_, event);
  }

  constexpr void on_triggered(event_type const &) noexcept(
      std::is_nothrow_invocable_v<Action>) requires std::invocable<Action> {
    std::invoke(action_);
  }

private : [[no_unique_address]] Action action_;
};

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, std::invocable<add_cref_t<Event>> Action>
constexpr action_transition<CurrentState, NextState, Event, Action>
action_transition_for(Action &&action) noexcept(
    std::is_nothrow_constructible_v<
        action_transition<CurrentState, NextState, Event, Action>>) {
  return action_transition<CurrentState, NextState, Event, Action>{
      std::forward<Action>(action)};
}

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event, std::invocable Action>
constexpr action_transition<CurrentState, NextState, Event, Action>
action_transition_for(Action &&action) noexcept(
    std::is_nothrow_constructible_v<
        action_transition<CurrentState, NextState, Event, Action>>) {
  return action_transition<CurrentState, NextState, Event, Action>{
      std::forward<Action>(action)};
}
} // namespace skizzay::fsm