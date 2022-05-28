#pragma once

#include "skizzay/fsm/event_dispatcher.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/optional_reference.h"
#include "skizzay/fsm/query.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/types.h"

#include <tuple>
#include <type_traits>

namespace skizzay::fsm {
namespace state_chart_details_ {
enum class status { stopped, stopping, starting, running };
} // namespace state_chart_details_

template <concepts::state_container RootStateContainer,
          concepts::transition_table TransitionTable>
requires contains_all_v<states_list_t<RootStateContainer>,
                        states_list_t<TransitionTable>> &&
    (length_v<states_list_t<RootStateContainer>> ==
     length_v<states_list_t<TransitionTable>>)struct state_chart {
  using transition_table_type = map_t<TransitionTable, std::remove_cvref_t>;

  constexpr explicit state_chart(
      RootStateContainer &&root_state_container,
      transition_table_type
          transition_table) noexcept(std::
                                         is_nothrow_move_constructible_v<
                                             transition_table_type>)
      : root_state_container_{std::forward<RootStateContainer>(
            root_state_container)},
        event_dispatcher_{std::move_if_noexcept(transition_table),
                          root_state_container_},
        status_{state_chart_details_::status::stopped} {}

        // TODO: Determine if we should stop the state chart in the constructor if it is still running

  template <concepts::event_in<transition_table_type> Event>
  constexpr bool on(Event &&event) {
    return is_running() &&
           event_dispatcher_.dispatch_event(std::forward<Event>(event));
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

  constexpr void start() {
    if (state_chart_details_::status::stopped == status_) {
      status_ = state_chart_details_::status::starting;
      event_dispatcher_.dispatch_event(initial_entry_event);
      status_ = state_chart_details_::status::running;
    }
  }

  constexpr void stop() {
    if (state_chart_details_::status::running == status_) {
      status_ = state_chart_details_::status::stopping;
      event_dispatcher_.dispatch_event(final_exit_event);
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
  RootStateContainer root_state_container_;
  event_dispatcher<transition_table_type, RootStateContainer> event_dispatcher_;
  state_chart_details_::status status_;
};

template <concepts::state_container RootStateContainer,
          concepts::transition_table TransitionTable>
state_chart(RootStateContainer &&, TransitionTable &&) -> state_chart<
    std::remove_reference_t<RootStateContainer>,
    std::remove_reference_t<map_t<TransitionTable, std::remove_cvref_t>>>;
} // namespace skizzay::fsm
