#pragma once

#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/trigger.h"
#include "skizzay/fsm/type_list.h"

#include <tuple>

namespace skizzay::fsm {

template <concepts::event_transition_context ParentEventTransitionContext,
          concepts::states_list StatesList>
struct event_context_node : ParentEventTransitionContext {
  using ParentEventTransitionContext::states_list_type;
  using ParentEventTransitionContext::transition_table_type;

  using ParentEventTransitionContext::ParentEventTransitionContext;

  constexpr explicit event_context_node(
      ParentEventTransitionContext const &
          parent_event_transition_context) noexcept(std::
                                                        is_nothrow_copy_constructible_v<
                                                            ParentEventTransitionContext>)
      : ParentEventTransitionContext(parent_event_transition_context) {}

  template <concepts::self_transition_in<
      transition_table_t<ParentEventTransitionContext>>
                Transition>
  constexpr void on_transition(Transition &transition,
                               event_t<Transition> const &event) {
    scheduled_ = true;
    trigger(transition, event);
    this->template schedule_entry<next_state_t<Transition>>();
  }

  template <
      concepts::transition_in<transition_table_t<ParentEventTransitionContext>>
          Transition>
  constexpr void on_transition(Transition &transition,
                               event_t<Transition> const &event) {
    scheduled_ = true;
    exiting_container_ =
        will_exit_container() ||
        !concepts::state_in<next_state_t<Transition>, StatesList>;
    ParentEventTransitionContext::on_transition(transition, event);
  }

  constexpr bool has_been_scheduled() const noexcept { return scheduled_; }

  constexpr bool will_exit_container() const noexcept {
    return exiting_container_;
  }

private:
  bool scheduled_ = false;
  bool exiting_container_ = false;
};
} // namespace skizzay::fsm
