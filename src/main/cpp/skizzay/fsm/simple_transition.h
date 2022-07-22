#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state.h"

#include <type_traits>

namespace skizzay::fsm {

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event>
struct simple_transition {
  using current_state_type = std::remove_cvref_t<CurrentState>;
  using next_state_type = std::remove_cvref_t<NextState>;
  using event_type = std::remove_cvref_t<Event>;
};
} // namespace skizzay::fsm