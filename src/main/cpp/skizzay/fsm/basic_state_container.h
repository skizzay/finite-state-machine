#pragma once

#include "skizzay/fsm/accepts.h"
#include "skizzay/fsm/enter.h"
#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/exit.h"
#include "skizzay/fsm/is_done.h"
#include "skizzay/fsm/optional_reference.h"
#include "skizzay/fsm/reenter.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/types.h"

#include <cassert>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>

namespace skizzay::fsm {

namespace basic_state_container_details_ {

template <typename Derived, concepts::state State>
requires std::same_as<State, std::remove_cvref_t<State>> &&
    (!concepts::state_container<State>)struct container {
  friend Derived;

  using states_list_type = states_list<State>;

  constexpr container() noexcept(
      std::is_nothrow_default_constructible_v<
          State>) requires(std::is_default_constructible_v<State>) = default;

  template <typename... Args>
  requires std::is_constructible_v<State, Args...>
  constexpr explicit container(Args &&...args) noexcept(
      std::is_nothrow_constructible_v<State, Args...>)
      : state_{std::forward<Args>(args)...} {}

  template <std::same_as<State> _ = State> constexpr bool is() const noexcept {
    return is_active();
  }

  template <std::same_as<State> _ = State>
  constexpr optional_reference<State const> current_state() const noexcept {
    using result_type = optional_reference<State const>;
    return is_active() ? result_type{this->template state<State>()}
                       : result_type{std::nullopt};
  }

  template <std::same_as<State> _ = State> constexpr State &state() noexcept {
    return state_;
  }

  template <std::same_as<State> _ = State>
  constexpr State const &state() const noexcept {
    return state_;
  }

  constexpr bool is_active() const noexcept { return active_; }

  constexpr bool is_inactive() const noexcept { return !active_; }

  template <concepts::query<states_list_type> Query>
  constexpr bool query(Query &&query) const
      noexcept(concepts::nothrow_query<Query, states_list_type>) {
    assert(is_active() || "Trying to query an inactive state container");
    std::invoke(std::forward<Query>(query), state_);
    return is_done(query);
  }

  constexpr State memento() const
      noexcept(std::is_nothrow_copy_constructible_v<State>) {
    return state_;
  }

  constexpr void on_entry(concepts::state_schedule auto const &,
                          initial_entry_event_t const &event,
                          concepts::event_engine auto event_engine,
                          concepts::state_provider auto state_provider) {
    activate(event, event_engine, state_provider);
  }

  constexpr void on_entry(concepts::state_schedule auto const &state_schedule,
                          concepts::event auto const &event,
                          concepts::event_engine auto event_engine,
                          concepts::state_provider auto state_provider) {
    if (state_schedule.template is_scheduled<state_t<Derived>>()) {
      activate(event, event_engine, state_provider);
    }
  }

  constexpr bool on_event(concepts::event_transition_context auto &,
                          final_exit_event_t const &event,
                          concepts::event_engine auto &event_engine,
                          concepts::state_provider auto &state_provider) {
    deactivate(event, event_engine, state_provider);
    return true;
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context,
           concepts::event auto const &event,
           concepts::event_engine auto &event_engine,
           concepts::state_provider auto &state_provider) {
    return attempt_transitions(
        event_transition_context, state(), event, state_provider,
        [&]() { deactivate(event, event_engine, state_provider); });
  }

private:
  bool active_ = false;
  State state_;

  constexpr Derived &derived() noexcept {
    return *static_cast<Derived *>(this);
  }

  constexpr Derived const &derived() const noexcept {
    return *static_cast<Derived const *>(this);
  }

  constexpr void activate(concepts::event auto const &event,
                          concepts::event_engine auto event_engine,
                          concepts::state_provider auto state_provider) {
    if (is_active()) {
      derived().do_reentry(event, event_engine, state_provider);
    } else {
      derived().do_entry(event, event_engine, state_provider);
      active_ = true;
    }
  }

  constexpr void deactivate(concepts::event auto const &event,
                            concepts::event_engine auto event_engine,
                            concepts::state_provider auto state_provider) {
    assert(is_active() && "Exiting inactive state");
    active_ = false;
    derived().do_exit(event, event_engine, state_provider);
  }

  constexpr void do_exit(concepts::event auto const &event,
                         concepts::event_engine auto &event_engine,
                         concepts::state_provider auto &state_provider) {
    exit(state(), event, event_engine, state_provider);
  }

  constexpr void do_entry(concepts::event auto const &event,
                          concepts::event_engine auto &event_engine,
                          concepts::state_provider auto &state_provider) {
    enter(state(), event, event_engine, state_provider);
  }

  constexpr void do_reentry(concepts::event auto const &event,
                            concepts::event_engine auto &event_engine,
                            concepts::state_provider auto &state_provider) {
    reenter(state(), event, event_engine, state_provider);
  }
};
} // namespace basic_state_container_details_

template <typename Derived, concepts::state State>
using basic_state_container =
    basic_state_container_details_::container<Derived, State>;
} // namespace skizzay::fsm