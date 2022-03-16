#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state.h"

namespace skizzay::fsm {

template <concepts::state CurrentState, concepts::state NextState,
          concepts::event Event>
struct simple_transition {
  using current_state_type = CurrentState;
  using next_state_type = NextState;
  using event_type = Event;
};
} // namespace skizzay::fsm