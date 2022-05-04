#pragma once

#include "skizzay/fsm/accepts.h"
#include "skizzay/fsm/enter.h"
#include "skizzay/fsm/entry_context.h"
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

struct state_transition_ambiguity : std::logic_error {
  using logic_error::logic_error;
};

namespace basic_state_container_details_ {

enum class acceptance_type { unaccepted, reentered, exited };

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

  constexpr void on_entry(concepts::initial_entry_context auto &entry_context) {
    activate(entry_context);
  }

  constexpr void on_entry(concepts::entry_context auto &entry_context) {
    if (entry_context.template is_scheduled<state_t<Derived>>()) {
      activate(entry_context);
    }
  }

  constexpr bool on_event(concepts::final_exit_event_transition_context auto
                              &final_exit_event_context) {
    deactivate(final_exit_event_context);
    return true;
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context) {
    switch (attempt_transitions(event_transition_context)) {
    case acceptance_type::reentered:
      return true;

    case acceptance_type::exited:
      deactivate(event_transition_context);
      return true;

    default:
      return false;
    }
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

  constexpr void activate(concepts::event_context auto &event_context) {
    if (is_active()) {
      derived().do_reentry(event_context);
    } else {
      derived().do_entry(event_context);
      active_ = true;
    }
  }

  constexpr void deactivate(concepts::event_context auto &event_context) {
    assert(is_active() && "Exiting inactive state");
    active_ = false;
    derived().do_exit(event_context);
  }

  constexpr void do_exit(concepts::event_context auto &event_context) {
    exit(state(), event_context);
  }

  constexpr void do_entry(concepts::event_context auto &event_context) {
    enter(state(), event_context);
  }

  constexpr void do_reentry(concepts::event_context auto &event_context) {
    reenter(state(), event_context);
  }

  constexpr concepts::transition_table auto get_transitions(
      concepts::event_transition_context auto &event_transition_context)
      const noexcept {
    return event_transition_context.get_transitions(state());
  }

  constexpr bool accepts_transition(
      concepts::transition auto const &transition,
      concepts::event_context auto const &event_context) const noexcept {
    return accepts(transition, state(), event_context);
  }

  constexpr acceptance_type attempt_transitions(
      concepts::event_transition_context auto &event_transition_context) {
    acceptance_type acceptance = acceptance_type::unaccepted;
    auto attempt_to_transition =
        [&]<concepts::transition Transition>(Transition &transition) {
          if (accepts_transition(std::as_const(transition),
                                 std::as_const(event_transition_context))) {
            if constexpr (concepts::self_transition<Transition>) {
              switch (acceptance) {
              case acceptance_type::exited:
                throw state_transition_ambiguity{
                    "Self transition encountered but state has already exited"};

              default:
                acceptance = acceptance_type::reentered;
              }
            } else {
              switch (acceptance) {
              case acceptance_type::reentered:
                throw state_transition_ambiguity{
                    "State changing transition encountered but state has "
                    "already reentered"};

              default:
                acceptance = acceptance_type::exited;
              }
            }
            event_transition_context.on_transition(transition);
          }
        };
    std::apply(
        [attempt_to_transition](concepts::transition auto &...transitions) {
          (attempt_to_transition(transitions), ...);
        },
        get_transitions(event_transition_context));
    return acceptance;
  }
};
} // namespace basic_state_container_details_

template <typename Derived, concepts::state State>
using basic_state_container =
    basic_state_container_details_::container<Derived, State>;
} // namespace skizzay::fsm