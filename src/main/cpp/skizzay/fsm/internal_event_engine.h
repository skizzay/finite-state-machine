#pragma once

#include "skizzay/fsm/event_transition_context_root.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/task_queue.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {
template <
    concepts::root_transition_table RootTransitionTable,
    concepts::root_state_container<RootTransitionTable> RootStateContainer>
struct internal_event_engine {
  using events_list_type = events_list_t<RootTransitionTable>;

  constexpr explicit internal_event_engine(
      RootTransitionTable &root_transition_table,
      RootStateContainer &root_state_container, task_queue &q) noexcept
      : root_transition_table_{root_transition_table},
        root_state_container_{root_state_container}, q_{q} {}

  constexpr void
  post_event(concepts::event_in<RootTransitionTable> auto &&event) {
    q_.push([this, event = std::forward<decltype(event)>(event)]() {
      dispatch_event(event);
    });
  }

private:
  RootTransitionTable &root_transition_table_;
  RootStateContainer &root_state_container_;
  task_queue &q_;

  constexpr void
  dispatch_event(concepts::event_in<RootTransitionTable> auto const &event) {
    event_transition_context_root event_transition_context{
        get_transition_table_for(root_transition_table_, event)};

    if (root_state_container_.on_event(event_transition_context, event, *this,
                                       root_state_container_)) {
      root_state_container_.on_entry(event_transition_context.state_schedule(),
                                     event, *this, root_state_container_);
    }
  }
};
} // namespace skizzay::fsm
