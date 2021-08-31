#pragma once

#include <cassert>
#include <iostream>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/overload.h>
#include <skizzay/fsm/traits.h>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace skizzay::fsm {

namespace flat_state_container_details_ {

[[noreturn]] inline void
on_bad_state(std::string_view const assertion_message) {
  std::cerr << assertion_message;
  std::terminate();
}

template <concepts::state_container... StateContainers> class container {
  using tuple_type = std::tuple<StateContainers...>;
  using variant_type = std::variant<std::monostate, StateContainers *...>;
  tuple_type state_containers_;
  variant_type current_state_container_;

public:
  using states_list_type = concat_t<states_list_t<StateContainers>...>;

  static_assert(0 < length_v<states_list_type>,
                "State container template argument list cannot be empty");
  // static_assert(length_v<states_list_type> ==
  //                   length_v<unique_t<states_list_type>>,
  //               "State containers cannot hold the same state");

  template <typename... Args>
  requires std::is_constructible_v<tuple_type, Args...>
  constexpr explicit container(Args &&...args) noexcept(
      std::is_nothrow_constructible_v<tuple_type, Args...>)
      : state_containers_{std::forward<Args>(args)...},
        current_state_container_{std::monostate{}} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return std::visit(
        overload([](std::monostate const) noexcept { return false; },
                 [](concepts::state_container_for<S> auto const
                        *const container) noexcept {
                   return container->template is<S>();
                 },
                 [](auto const *const) noexcept { return false; }),
        current_state_container_);
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return std::visit(
        overload(
            []<typename Container>(Container const &container) noexcept
            -> optional_reference<S const> {
              if constexpr (contains_v<states_list_t<Container>, S>) {
                return container.template current_state<S>();
              } else {
                return std::nullopt;
              }
            },
            [](std::monostate const) noexcept -> optional_reference<S const> {
              return std::nullopt;
            }),
        current_state_container_);
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &any_state() noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template any_state<S>();
  }

  template <concepts::state_in<states_list_type> S, concepts::ancestry Ancestry>
  constexpr auto ancestry_to(Ancestry ancestry) noexcept {
    return std::get<state_container_for_t<tuple_type, S>>(state_containers_)
        .template ancestry_to<S>(ancestry);
  }

  template <concepts::machine Machine>
  constexpr void on_initial_entry(Machine &machine) {
    assert(std::holds_alternative<std::monostate>(current_state_container_) &&
           "Initial entry into a state while already in a valid state");
    auto &container = std::get<0>(state_containers_);
    current_state_container_ = std::addressof(container);
    container.on_initial_entry(machine);
  }

  template <concepts::machine Machine>
  constexpr void on_final_exit(Machine &machine) {
    std::visit(overload(
                   [](std::monostate const) noexcept {
                     on_bad_state("on_final_exit invoked when not in a state");
                   },
                   [&machine](auto *const container) {
                     container->on_final_exit(machine);
                   }),
               current_state_container_);
    current_state_container_ = std::monostate{};
  }

  template <concepts::state_in<states_list_type> CurrentState,
            concepts::state NextState, concepts::machine Machine,
            concepts::event Event>
  constexpr void on_exit(Machine &machine, Event const &event) {
    std::visit(
        overload(
            [](std::monostate const) noexcept {
              on_bad_state("Exiting a state when not in a valid state");
            },
            [](auto const *const) noexcept {
              on_bad_state("Exiting a state that we're not in");
            },
            [&machine, &event](concepts::state_container_for<CurrentState> auto
                                   *const container) {
              container->template on_exit<CurrentState, NextState>(machine,
                                                                   event);
            }),
        current_state_container_);
    current_state_container_ = std::monostate{};
  }

  template <concepts::state PreviousState,
            concepts::state_in<states_list_type> CurrentState,
            concepts::machine Machine, concepts::event Event>
  constexpr void on_entry(Machine &machine, Event const &event) {
    assert(std::holds_alternative<std::monostate>(current_state_container_) &&
           "Entering a state without previously exiting from previous state");
    using target_container_type =
        front_t<containers_containing_state_t<tuple_type, CurrentState>>;
    target_container_type &container =
        std::get<target_container_type>(state_containers_);
    current_state_container_ = std::addressof(container);
    container.template on_entry<PreviousState, CurrentState>(machine, event);
  }

  template <concepts::state_in<states_list_type> S, concepts::machine Machine,
            concepts::event Event>
  constexpr void on_reentry(Machine &machine, Event const &event) {
    std::visit(
        overload(
            [](std::monostate const) noexcept {
              on_bad_state("on_reentry called when not in a state");
            },
            []<typename Container>(Container *const) {
              on_bad_state("on_reentry called when requested state is not "
                           "current state");
            },
            [&machine,
             &event](concepts::state_container_for<S> auto *const container) {
              container->template on_reentry<S>(machine, event);
            }),
        current_state_container_);
  }

  template <concepts::machine Machine,
            concepts::transition_coordinator TransitionCoordinator>
  constexpr bool
  schedule_acceptable_transitions(Machine const &machine,
                                  TransitionCoordinator &coordinator) const {
    return std::visit(
        overload(
            [](std::monostate const) noexcept {
              on_bad_state(
                  "schedule_acceptable_transitions called when not in a state");
            },
            [&machine, &coordinator]<typename Container>(
                Container const *const container) {
              return container->schedule_acceptable_transitions(machine,
                                                                coordinator);
            }),
        current_state_container_);
  }
};
} // namespace flat_state_container_details_

template <concepts::state_container... StateContainers>
using flat_state =
    flat_state_container_details_::container<StateContainers...>;

} // namespace skizzay::fsm