#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/transition.h"

#include <concepts>

namespace skizzay::fsm {

namespace trigger_details_ {
template <typename... Ts> void on_triggered(Ts const &...) = delete;

struct trigger_fn final {
  template <concepts::transition Transition>
  constexpr void operator()(Transition &,
                            event_t<Transition> const &) const noexcept {}

  template <concepts::transition Transition>
  requires requires(Transition &t, event_t<Transition> const &ec) {
    t.on_triggered(ec);
  }
  constexpr void operator()(Transition &transition,
                            event_t<Transition> const &event) const {
    transition.on_triggered(event);
  }

  template <concepts::transition Transition>
  requires requires(Transition &t, event_t<Transition> const &ec) {
    on_triggered(t, ec);
  }
  constexpr void operator()(Transition &transition,
                            event_t<Transition> const &event) const {
    on_triggered(transition, event);
  }

  template <concepts::transition Transition>
  requires requires(event_t<Transition> const &ec) { ec.on_triggered(); }
  constexpr void operator()(Transition &,
                            event_t<Transition> const &event) const {
    event.on_triggered();
  }

  template <concepts::transition Transition>
  requires requires(event_t<Transition> const &ec) { on_triggered(ec); }
  constexpr void operator()(Transition &,
                            event_t<Transition> const &event) const {
    on_triggered(event);
  }
};
} // namespace trigger_details_

inline namespace trigger_ns_ {
inline constexpr trigger_details_::trigger_fn trigger = {};
}
} // namespace skizzay::fsm