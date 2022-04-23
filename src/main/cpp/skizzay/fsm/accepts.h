#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/types.h"

namespace skizzay::fsm {
namespace accepts_details_ {
template <typename... Ts> void is_accepted(Ts const &...) = delete;

template <typename Transition, typename EventContext>
concept transition_acceptable = concepts::transition<Transition> &&
    concepts::event_context_for<EventContext, event_t<Transition>> &&(
        requires(Transition const &transition,
                 current_state_t<Transition> const &state,
                 EventContext const &event_context) {
          { transition.is_accepted(state, event_context) }
          noexcept->concepts::boolean;
        } ||
        requires(Transition const &transition,
                 current_state_t<Transition> const &state,
                 EventContext const &event_context) {
          { is_accepted(transition, state, event_context) }
          noexcept->concepts::boolean;
        });

template <typename State, typename EventContext>
concept transitionable_state_requiring_entire_event_context =
    concepts::state<State> && concepts::event_context<EventContext> &&(
        requires(State const &state, EventContext const &event_context) {
          { state.is_accepted(event_context) }
          noexcept->concepts::boolean;
        } ||
        requires(State const &state, EventContext const &event_context) {
          { is_accepted(state, event_context) }
          noexcept->concepts::boolean;
        });

struct accepts_fn final {
  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  constexpr bool operator()(Transition const &,
                            current_state_t<Transition> const &,
                            EventContext const &) const noexcept {
    return true;
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(Transition const &t, current_state_t<Transition> const &s,
                    EventContext const &e) {
    { t.is_accepted(s, e) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(Transition const &transition,
                            current_state_t<Transition> const &state,
                            EventContext const &event_context) const noexcept {
    return transition.is_accepted(state, event_context);
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(Transition const &t, current_state_t<Transition> const &s,
                    EventContext const &e) {
    { is_accepted(t, s, e) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(Transition const &transition,
                            current_state_t<Transition> const &state,
                            EventContext const &event_context) const noexcept {
    return is_accepted(transition, state, event_context);
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(current_state_t<Transition> const &s,
                    EventContext const &e) {
    { s.is_accepted(e) }
    noexcept->concepts::boolean;
  } &&(!transition_acceptable<Transition, EventContext>)constexpr bool
  operator()(Transition const &, current_state_t<Transition> const &state,
             EventContext const &event_context) const noexcept {
    return state.is_accepted(event_context);
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(current_state_t<Transition> const &s,
                    EventContext const &e) {
    { is_accepted(s, e) }
    noexcept->concepts::boolean;
  } &&(!transition_acceptable<Transition, EventContext>)constexpr bool
  operator()(Transition const &, current_state_t<Transition> const &state,
             EventContext const &event_context) const noexcept {
    return is_accepted(state, event_context);
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(current_state_t<Transition> const &s,
                    EventContext const &e) {
    { s.is_accepted(e.event()) }
    noexcept->concepts::boolean;
  } &&(!transition_acceptable<Transition, EventContext> &&
       !transitionable_state_requiring_entire_event_context<
           current_state_t<Transition>, EventContext>)constexpr bool
  operator()(Transition const &, current_state_t<Transition> const &state,
             EventContext const &event_context) const noexcept {
    return state.is_accepted(event_context.event());
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(current_state_t<Transition> const &s,
                    EventContext const &e) {
    { is_accepted(s, e.event()) }
    noexcept->concepts::boolean;
  } &&(!transition_acceptable<Transition, EventContext> &&
       !transitionable_state_requiring_entire_event_context<
           current_state_t<Transition>, EventContext>)constexpr bool
  operator()(Transition const &, current_state_t<Transition> const &state,
             EventContext const &event_context) const noexcept {
    return is_accepted(state, event_context.event());
  }
};
} // namespace accepts_details_

inline namespace accepts_fn_ {
inline constexpr accepts_details_::accepts_fn accepts = {};
} // namespace accepts_fn_

} // namespace skizzay::fsm
