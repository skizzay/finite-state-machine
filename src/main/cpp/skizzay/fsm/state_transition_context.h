#pragma once

#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/event_provider.h"
#include "skizzay/fsm/state_provider.h"

#include <type_traits>

namespace skizzay::fsm {
template <typename T>
struct is_state_transition_context
    : std::conjunction<is_event_engine<T>, is_event_provider<T>,
                       is_state_provider<T>> {};

namespace concepts {
template <typename T>
concept state_transition_context = is_state_transition_context<T>::value;
} // namespace concepts

} // namespace skizzay::fsm
