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
template <typename> struct template_member_function {
  template <typename> struct schedule_entry : std::false_type {};
};

template <typename T> template <concepts::state State>
requires requires(T &t) { t.template schedule_entry<State>(); }
struct template_member_function<T>::schedule_entry<State> : std::true_type {};

} // namespace event_context_node_details_

template <concepts::state_container StateContainer,
          concepts::event_context ParentEventContext>
requires all_v<
    next_states_list_t<ParentEventContext>,
    event_context_node_details_::template_member_function<
        ParentEventContext>::template schedule_entry> struct event_context_node
    : ParentEventContext {
  using ParentEventContext::event_type;
  using ParentEventContext::events_list_type;
  using ParentEventContext::states_list_type;
  using ParentEventContext::transition_table_type;

  constexpr explicit event_context_node(
      StateContainer &state_container,
      ParentEventContext &parent_event_context) noexcept
      : ParentEventContext{parent_event_context}, state_container_{
                                                      state_container} {}

  template <concepts::transition_in<transition_table_t<ParentEventContext>>
                Transition>
  requires contains_v<states_list_t<StateContainer>, next_state_t<Transition>>
  constexpr void on_transition(Transition &transition) {
    scheduled_ = true;
    trigger(transition, *this);
    this->template schedule_entry<next_state_t<Transition>>();
  }

  template <concepts::transition_in<transition_table_t<ParentEventContext>>
                Transition>
  constexpr void on_transition(Transition &transition) {
    scheduled_ = true;
    exiting_container_ = true;
    ParentEventContext::on_transition(transition);
  }

  constexpr bool has_been_scheduled() const noexcept { return scheduled_; }

  constexpr bool will_exit_container() const noexcept {
    return exiting_container_;
  }

private:
  StateContainer &state_container_;
  bool scheduled_ = false;
  bool exiting_container_ = false;
};
} // namespace skizzay::fsm
