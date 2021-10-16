#pragma once

#include <concepts>
#include <skizzay/fsm/concepts.h>

namespace skizzay::fsm {

namespace trigger_details_ {
struct trigger_fn final {
  template <concepts::actionable_transition Transition,
            std::convertible_to<event_t<Transition>> Event>
  constexpr void operator()(Transition &transition, Event const &event) const
      noexcept(noexcept(transition.on_triggered(event))) {
    transition.on_triggered(event);
  }

  template <concepts::transition Transition,
            std::convertible_to<event_t<Transition>> Event>
  constexpr void operator()(Transition &, Event const &) const noexcept {}
};
} // namespace trigger_details_

inline namespace trigger_ns_ {
inline constexpr trigger_details_::trigger_fn trigger = {};
}
} // namespace skizzay::fsm