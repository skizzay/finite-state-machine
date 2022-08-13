#pragma once

#include "skizzay/fsm/event_transition_context_root.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/internal_event_engine.h"
#include "skizzay/fsm/optional_reference.h"
#include "skizzay/fsm/query.h"
#include "skizzay/fsm/snapshot.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/task_queue.h"
#include "skizzay/fsm/transition_table.h"

#include <tuple>
#include <type_traits>

namespace skizzay::fsm {
namespace state_chart_details_ {
enum class status { stopped, stopping, starting, running };
} // namespace state_chart_details_

template <
    concepts::root_transition_table RootTransitionTable,
    concepts::root_state_container<RootTransitionTable> RootStateContainer>
struct state_chart {
  using transition_table_type = RootTransitionTable;

  constexpr explicit state_chart(
      transition_table_type transition_table,
      RootStateContainer
          root_state_container) noexcept(std::
                                             is_nothrow_move_constructible_v<
                                                 transition_table_type> &&std::
                                                 is_nothrow_move_constructible_v<
                                                     RootStateContainer>)
      : root_transition_table_{std::move(transition_table)},
        root_state_container_{std::move(root_state_container)},
        status_{state_chart_details_::status::stopped} {}

  // TODO: Determine if we should stop the state chart in the destructor if it
  // is still running

  constexpr bool
  on(concepts::event_in<transition_table_type> auto const &event) {
    return is_running() && handle_event(event);
  }

  template <
      concepts::state_in<current_states_list_t<transition_table_type>> State>
  constexpr bool is() const noexcept {
    return is_running() && root_state_container_.template is<State>();
  }

  template <
      concepts::state_in<current_states_list_t<transition_table_type>> State>
  constexpr optional_reference<State const> current_state() const noexcept {
    return root_state_container_.template current_state<State>();
  }

  template <concepts::query<current_states_list_t<transition_table_type>> Query>
  constexpr void query(Query &&query) const
      noexcept(concepts::nothrow_query<
               Query, current_states_list_t<transition_table_type>>) {
    if (is_running()) {
      root_state_container_.query(std::forward<Query>(query));
    }
  }

  constexpr memento_t<RootStateContainer> memento() const
      noexcept(is_memento_nothrow_v<RootStateContainer>) {
    using skizzay::fsm::memento;
    return memento(root_state_container_);
  }

  constexpr void
  recover_from_memento(memento_t<RootStateContainer> &&memento) noexcept(
      is_recover_from_memento_nothrow_v<RootStateContainer>) {
    using skizzay::fsm::recover_from_memento;
    recover_from_memento(root_state_container_, std::move(memento));
  }

  constexpr void start() {
    if (state_chart_details_::status::stopped == status_) {
      status_ = state_chart_details_::status::starting;
      handle_event(initial_entry_event);
      status_ = state_chart_details_::status::running;
    }
  }

  constexpr void stop() {
    if (state_chart_details_::status::running == status_) {
      status_ = state_chart_details_::status::stopping;
      handle_event(final_exit_event);
      status_ = state_chart_details_::status::stopped;
    }
  }

  constexpr bool is_running() const noexcept {
    return state_chart_details_::status::running == status_;
  }

  constexpr bool is_stopped() const noexcept {
    return state_chart_details_::status::stopped == status_;
  }

private:
  [[no_unique_address]] transition_table_type root_transition_table_;
  RootStateContainer root_state_container_;
  // Oh the irony... we need this to track the state machine's state.
  state_chart_details_::status status_;

  constexpr bool handle_event(concepts::event auto const &event) {
    task_queue q;
    event_transition_context_root transition_context{
        get_transition_table_for(root_transition_table_, event)};
    internal_event_engine event_engine{root_transition_table_,
                                       root_state_container_, q};

    if (root_state_container_.on_event(transition_context, event, event_engine,
                                       root_state_container_)) {
      root_state_container_.on_entry(transition_context.state_schedule(), event,
                                     event_engine, root_state_container_);
      q.run();
      return true;
    } else {
      return false;
    }
  }

  template <
      concepts::event_in<events_list<initial_entry_event_t, final_exit_event_t>>
          Event>
  constexpr void handle_event(Event const &) {
    task_queue q;
    internal_event_engine event_engine{root_transition_table_,
                                       root_state_container_, q};
    if constexpr (std::same_as<Event, initial_entry_event_t>) {
      execute_initial_entry(root_state_container_, event_engine,
                            root_state_container_);
    } else {
      execute_final_exit(root_state_container_, event_engine,
                         root_state_container_);
    }
    q.run();
  }
};

template <
    concepts::root_transition_table RootTransitionTable,
    concepts::root_state_container<RootTransitionTable> RootStateContainer>
state_chart(RootTransitionTable &&, RootStateContainer &&)
    -> state_chart<std::remove_reference_t<RootTransitionTable>,
                   std::remove_reference_t<RootStateContainer>>;
} // namespace skizzay::fsm
