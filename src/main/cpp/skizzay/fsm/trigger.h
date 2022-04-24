#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/types.h"

#include <concepts>

namespace skizzay::fsm {

namespace trigger_details_ {
template <typename... Ts> void on_triggered(Ts const &...) = delete;

template <typename Transition, typename EventContext>
concept trigger_requires_only_event = requires(Transition &transition,
                                               EventContext &event_context) {
  transition.on_triggered(std::as_const(event_context).event());
}
|| requires(Transition &transition, EventContext &event_context) {
  on_triggered(transition, std::as_const(event_context).event());
};

struct trigger_fn final {
  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  constexpr void operator()(Transition &, EventContext &) const noexcept {}

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(Transition &transition, EventContext &event_context) {
    transition.on_triggered(std::as_const(event_context).event());
  }
  constexpr void operator()(Transition &transition,
                            EventContext &event_context) const
      noexcept(noexcept(
          transition.on_triggered(std::as_const(event_context).event()))) {
    transition.on_triggered(std::as_const(event_context).event());
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires requires(Transition &transition, EventContext &event_context) {
    on_triggered(transition, std::as_const(event_context).event());
  }
  constexpr void operator()(Transition &transition,
                            EventContext &event_context) const
      noexcept(noexcept(on_triggered(transition,
                                     std::as_const(event_context).event()))) {
    on_triggered(transition, std::as_const(event_context).event());
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires(!trigger_requires_only_event<Transition, EventContext>) &&
      requires(Transition &transition, EventContext &event_context) {
    transition.on_triggered(event_context);
  }
  constexpr void operator()(Transition &transition,
                            EventContext &event_context) const
      noexcept(noexcept(transition.on_triggered(event_context))) {
    transition.on_triggered(event_context);
  }

  template <concepts::transition Transition,
            concepts::event_context_for<event_t<Transition>> EventContext>
  requires(!trigger_requires_only_event<Transition, EventContext>) &&
      requires(Transition &transition, EventContext &event_context) {
    on_triggered(transition, event_context);
  }
  constexpr void operator()(Transition &transition,
                            EventContext &event_context) const
      noexcept(noexcept(on_triggered(transition, event_context))) {
    on_triggered(transition, event_context);
  }
};
} // namespace trigger_details_

inline namespace trigger_ns_ {
inline constexpr trigger_details_::trigger_fn trigger = {};
}
} // namespace skizzay::fsm