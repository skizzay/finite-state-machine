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

  constexpr void do_exit(concepts::event_context auto &) noexcept {}
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

  template <concepts::entry_context EntryContext>
  constexpr void enter_parent_state_container(EntryContext &entry_context) {
    using candidate_states_list_type =
        candidate_states_list_t<EntryContext,
                                parent_state_container<ParentState>>;
    if constexpr (!empty_v<candidate_states_list_type>) {
      if (has_state_scheduled_for_entry(
              std::as_const(entry_context),
              std::as_const(parent_state_container_))) {
        parent_state_container_.on_entry(entry_context);
      } else if (parent_state_container_.is_inactive()) {
        execute_initial_entry(entry_context, parent_state_container_);
      }
    } else if (parent_state_container_.is_inactive()) {
      execute_initial_entry(entry_context, parent_state_container_);
    }
  }

  template <concepts::entry_context EntryContext>
  constexpr void enter_child_state_container(EntryContext &entry_context) {
    using candidate_states_list_type =
        candidate_states_list_t<EntryContext, ChildStateContainer>;
    if constexpr (!empty_v<candidate_states_list_type>) {
      if (has_state_scheduled_for_entry(
              std::as_const(entry_context),
              std::as_const(child_state_container_))) {
        child_state_container_.on_entry(entry_context);
      } else {
        execute_initial_entry(entry_context, child_state_container_);
      }
    } else {
      execute_initial_entry(entry_context, child_state_container_);
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

  template <concepts::state_in<states_list_type> S>
  constexpr S const &state() const noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  template <concepts::state_in<states_list_type> S>
  constexpr S &state() noexcept {
    return this->template get_state_container_for<S>().template state<S>();
  }

  constexpr bool is_active() const noexcept {
    return child_state_container_.is_active();
  }

  constexpr bool is_inactive() const noexcept {
    return parent_state_container_.is_inactive();
  }

  constexpr void
  on_entry(concepts::initial_entry_context auto &initial_entry_context) {
    parent_state_container_.on_entry(initial_entry_context);
    child_state_container_.on_entry(initial_entry_context);
  }

  template <concepts::entry_context EntryContext>
  constexpr void on_entry(EntryContext &entry_context) {
    enter_parent_state_container(entry_context);
    enter_child_state_container(entry_context);
  }

  constexpr bool on_event(concepts::final_exit_event_transition_context auto
                              &event_transition_context) {
    bool const child_result =
        child_state_container_.on_event(event_transition_context);
    bool const parent_result =
        parent_state_container_.on_event(event_transition_context);
    exit(parent_state_container_.state(), event_transition_context);
    return child_result || parent_result;
  }

  constexpr bool
  on_event(concepts::event_transition_context auto &event_transition_context) {
    event_context_node child_event_context_node{*this,
                                                event_transition_context};
    if (child_state_container_.on_event(child_event_context_node)) {
      if (child_event_context_node.will_exit_container()) {
        auto [final_event_transition_context, ignored_] = execute_final_exit(
            event_transition_context, parent_state_container_);
        exit(parent_state_container_.state(), final_event_transition_context);
      }
      return true;
    } else {
      event_context_node parent_event_context_node{*this,
                                                   event_transition_context};
      if (parent_state_container_.on_event(parent_event_context_node)) {
        execute_final_exit(event_transition_context, child_state_container_);
        if (parent_event_context_node.will_exit_container()) {
          exit(parent_state_container_.state(), event_transition_context);
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