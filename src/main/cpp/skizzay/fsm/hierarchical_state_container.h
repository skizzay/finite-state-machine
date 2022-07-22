#pragma once

#include "skizzay/fsm/basic_state_container.h"
#include "skizzay/fsm/event_context_node.h"
#include "skizzay/fsm/exit.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/type_list.h"
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace hierarchical_state_container_details_ {

template <concepts::state ParentState>
struct parent_state_container
    : basic_state_container<parent_state_container<ParentState>, ParentState> {

  using basic_state_container<parent_state_container<ParentState>,
                              ParentState>::basic_state_container;

  constexpr void do_exit(concepts::event auto const &,
                         concepts::event_engine auto &,
                         concepts::state_provider auto &) noexcept {}
};

template <concepts::state ParentState,
          concepts::state_container ChildStateContainer>
class container {
  parent_state_container<ParentState> parent_state_container_;
  ChildStateContainer child_state_container_;

  template <concepts::state State>
  constexpr auto &get_state_container_for() noexcept {
    if constexpr (std::same_as<ParentState, State>) {
      return parent_state_container_;
    } else {
      return child_state_container_;
    }
  }

  template <concepts::state State>
  constexpr auto const &get_state_container_for() const noexcept {
    if constexpr (std::same_as<ParentState, State>) {
      return parent_state_container_;
    } else {
      return child_state_container_;
    }
  }

  constexpr void enter_child_state_container(
      concepts::state_schedule auto const &state_schedule,
      concepts::event auto const &event,
      concepts::event_engine auto &event_engine,
      concepts::state_provider auto &state_provider) {
    using candidate_states_list_type = intersection_of_t<
        next_states_list_t<std::remove_cvref_t<decltype(state_schedule)>>,
        states_list_t<ChildStateContainer>>;
    if constexpr (!empty_v<candidate_states_list_type>) {
      if (has_state_scheduled_for_entry(
              state_schedule, std::as_const(child_state_container_))) {
        child_state_container_.on_entry(state_schedule, event, event_engine,
                                        state_provider);
      } else {
        execute_initial_entry(child_state_container_, event_engine,
                              state_provider);
      }
    } else {
      execute_initial_entry(child_state_container_, event_engine,
                            state_provider);
    }
  }

  constexpr void enter_parent_state_container(
      concepts::state_schedule auto const &state_schedule,
      concepts::event auto const &event,
      concepts::event_engine auto &event_engine,
      concepts::state_provider auto &state_provider) {
    using candidate_states_list_type = intersection_of_t<
        next_states_list_t<std::remove_cvref_t<decltype(state_schedule)>>,
        states_list<ParentState>>;
    if constexpr (!empty_v<candidate_states_list_type>) {
      if (has_state_scheduled_for_entry(
              state_schedule, std::as_const(parent_state_container_))) {
        parent_state_container_.on_entry(state_schedule, event, event_engine,
                                         state_provider);
      } else if (parent_state_container_.is_inactive()) {
        execute_initial_entry(parent_state_container_, event_engine,
                              state_provider);
      }
    } else if (parent_state_container_.is_inactive()) {
      execute_initial_entry(parent_state_container_, event_engine,
                            state_provider);
    }
  }

public:
  using states_list_type =
      concat_t<states_list<ParentState>, states_list_t<ChildStateContainer>>;

  constexpr container() noexcept(
      std::conjunction_v<
          std::is_nothrow_default_constructible<
              parent_state_container<ParentState>>,
          std::is_nothrow_default_constructible<ChildStateContainer>>) requires
      std::is_default_constructible_v<ParentState> &&
      std::is_default_constructible_v<ChildStateContainer>
      : parent_state_container_{}, child_state_container_{} {}

  constexpr explicit container(ParentState parent_state) noexcept(
      std::conjunction_v<
          std::is_nothrow_move_constructible<
              parent_state_container<ParentState>>,
          std::is_nothrow_default_constructible<ChildStateContainer>>)
      : parent_state_container_{std::move_if_noexcept(parent_state)},
        child_state_container_{} {}

  constexpr explicit container(ChildStateContainer child_state_container) noexcept(
      std::conjunction_v<
          std::is_nothrow_default_constructible<
              parent_state_container<ParentState>>,
          std::is_nothrow_move_constructible<ChildStateContainer>>)
      : parent_state_container_{}, child_state_container_{std::move_if_noexcept(
                                       child_state_container)} {}

  constexpr explicit container(
      ParentState parent_state,
      ChildStateContainer
          child_state_container) noexcept(std::
                                              conjunction_v<
                                                  std::is_nothrow_constructible<
                                                      parent_state_container<
                                                          ParentState>,
                                                      ParentState &&>,
                                                  std::
                                                      is_nothrow_move_constructible<
                                                          ChildStateContainer>>)
      : parent_state_container_{std::move_if_noexcept(parent_state)},
        child_state_container_{std::move_if_noexcept(child_state_container)} {}

  template <concepts::state_in<states_list_type> S>
  constexpr bool is() const noexcept {
    return is_active() &&
           this->template get_state_container_for<S>().template is<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr optional_reference<S const> current_state() const noexcept {
    return this->template get_state_container_for<S>()
        .template current_state<S>();
  }

  template <concepts::query<states_list_type> Query>
  constexpr bool query(Query &&query) const
      noexcept(concepts::nothrow_query<Query, states_list_type>) {
    return parent_state_container_.query(std::forward<Query>(query)) ||
           child_state_container_.query(std::forward<Query>(query));
  }

  constexpr auto memento() const
      noexcept(is_memento_nothrow_v<parent_state_container<ParentState>>
                   &&is_memento_nothrow_v<ChildStateContainer>) {
    return std::tuple{parent_state_container_.memento(),
                      child_state_container_.memento()};
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S const &state() const noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &state() noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  constexpr bool is_active() const noexcept {
    return parent_state_container_.is_active() || child_state_container_.is_active();
  }

  constexpr bool is_inactive() const noexcept {
    return parent_state_container_.is_inactive() && child_state_container_.is_inactive();
  }

  constexpr void on_entry(concepts::state_schedule auto const &state_schedule,
                          initial_entry_event_t const &event,
                          concepts::event_engine auto &event_engine,
                          concepts::state_provider auto &state_provider) {
    parent_state_container_.on_entry(state_schedule, event, event_engine,
                                     state_provider);
    child_state_container_.on_entry(state_schedule, event, event_engine,
                                    state_provider);
  }

  constexpr void on_entry(concepts::state_schedule auto const &state_schedule,
                          concepts::event auto const &event,
                          concepts::event_engine auto &event_engine,
                          concepts::state_provider auto &state_provider) {
    enter_parent_state_container(state_schedule, event, event_engine,
                                 state_provider);
    enter_child_state_container(state_schedule, event, event_engine,
                                state_provider);
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context,
           final_exit_event_t const &event,
           concepts::event_engine auto &event_engine,
           concepts::state_provider auto &state_provider) {
    bool const child_result = child_state_container_.on_event(
        event_transition_context, event, event_engine, state_provider);
    bool const parent_result = parent_state_container_.on_event(
        event_transition_context, event, event_engine, state_provider);
    exit(parent_state_container_.state(), final_exit_event, event_engine,
         state_provider);
    return child_result || parent_result;
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context,
           concepts::event auto const &event,
           concepts::event_engine auto &event_engine,
           concepts::state_provider auto &state_provider) {
    event_context_node<std::remove_cvref_t<decltype(event_transition_context)>,
                       states_list_type>
        child_event_context_node{event_transition_context};
    if (child_state_container_.on_event(child_event_context_node, event,
                                        event_engine, state_provider)) {
      if (child_event_context_node.will_exit_container()) {
        execute_final_exit(parent_state_container_, event_engine,
                           state_provider);
        exit(parent_state_container_.state(), final_exit_event, event_engine,
             state_provider);
      }
      return true;
    } else {
      event_context_node<
          std::remove_cvref_t<decltype(event_transition_context)>,
          states_list_type>
          parent_event_context_node{event_transition_context};
      if (parent_state_container_.on_event(parent_event_context_node, event,
                                           event_engine, state_provider)) {
        execute_final_exit(child_state_container_, event_engine,
                           state_provider);
        if (parent_event_context_node.will_exit_container()) {
          exit(parent_state_container_.state(),
               event_transition_context.event(), event_engine, state_provider);
        }
        return true;
      } else {
        return false;
      }
    }
  }
};
} // namespace hierarchical_state_container_details_

template <concepts::state State>
struct basic_state_t<
    hierarchical_state_container_details_::parent_state_container<State>> {
  using type = State;
};

template <concepts::state ParentState,
          concepts::state_container ChildStateContainer>
using hierarchical_states =
    hierarchical_state_container_details_::container<ParentState,
                                                     ChildStateContainer>;

} // namespace skizzay::fsm