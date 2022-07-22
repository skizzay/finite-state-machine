#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/trigger.h"
#include "skizzay/fsm/type_list.h"
#include "skizzay/fsm/types.h"

#include <tuple>

namespace skizzay::fsm {

namespace event_context_node_details_ {
template <typename, typename>
struct schedule_entry_template_member_function : std::false_type {};
template <typename T, concepts::state State>
requires requires(T &t) { t.template schedule_entry<State>(); }
struct schedule_entry_template_member_function<T, State> : std::true_type {};

} // namespace event_context_node_details_

template <concepts::event_transition_context ParentEventTransitionContext,
          concepts::states_list StatesList>
requires all_v<
    next_states_list_t<ParentEventTransitionContext>,
    curry<
        event_context_node_details_::schedule_entry_template_member_function,
        ParentEventTransitionContext>::template type> struct event_context_node
    : ParentEventTransitionContext {
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
