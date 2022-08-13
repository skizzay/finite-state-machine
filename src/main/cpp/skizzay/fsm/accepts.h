#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/transition.h"

namespace skizzay::fsm {
namespace accepts_details_ {
template <typename... Ts> void is_accepted(Ts const &...) = delete;

struct accepts_fn final {
  template <concepts::transition Transition,
            concepts::state_provider StateProvider>
  constexpr bool operator()(Transition const &,
                            current_state_t<Transition> const &,
                            event_t<Transition> const &,
                            StateProvider const &) const noexcept {
    return true;
  }

  template <concepts::transition Transition,
            concepts::state_provider StateProvider>
  requires requires(Transition const &t, current_state_t<Transition> const &s,
                    event_t<Transition> const &e, StateProvider const &sp) {
    { t.is_accepted(s, e, sp) }
    noexcept->concepts::boolean;
  }
  constexpr bool
  operator()(Transition const &transition,
             current_state_t<Transition> const &state,
             event_t<Transition> const &event,
             StateProvider const &state_provider) const noexcept {
    return transition.is_accepted(state, event, state_provider);
  }

  template <concepts::transition Transition,
            concepts::state_provider StateProvider>
  requires requires(Transition const &t, current_state_t<Transition> const &s,
                    event_t<Transition> const &e, StateProvider const &sp) {
    { is_accepted(t, s, e, sp) }
    noexcept->concepts::boolean;
  }
  constexpr bool
  operator()(Transition const &transition,
             current_state_t<Transition> const &state,
             event_t<Transition> const &event,
             StateProvider const &state_provider) const noexcept {
    return is_accepted(transition, state, event, state_provider);
  }

  template <concepts::transition Transition,
            concepts::state_provider StateProvider>
  requires requires(Transition const &t, current_state_t<Transition> const &s,
                    event_t<Transition> const &e) {
    { t.is_accepted(s, e) }
    noexcept->concepts::boolean;
  }
  constexpr bool
  operator()(Transition const &transition,
             current_state_t<Transition> const &state,
             event_t<Transition> const &event,
             StateProvider const &) const noexcept {
    return transition.is_accepted(state, event);
  }

  template <concepts::transition Transition,
            concepts::state_provider StateProvider>
  requires requires(Transition const &t, current_state_t<Transition> const &s,
                    event_t<Transition> const &e) {
    { is_accepted(t, s, e) }
    noexcept->concepts::boolean;
  }
  constexpr bool
  operator()(Transition const &transition,
             current_state_t<Transition> const &state,
             event_t<Transition> const &event,
             StateProvider const &) const noexcept {
    return is_accepted(transition, state, event);
  }
};
} // namespace accepts_details_

inline namespace accepts_fn_ {
inline constexpr accepts_details_::accepts_fn accepts = {};
} // namespace accepts_fn_

} // namespace skizzay::fsm
