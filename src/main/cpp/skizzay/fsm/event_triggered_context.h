#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/event_provider.h"

namespace skizzay::fsm {
namespace concepts {

template <typename T>
concept event_triggered_context = event_engine<T> && event_provider<T>;

template <typename T, typename... Events>
concept event_triggered_context_for =
    (event<Events> && ...) &&
    (event_engine_for<T, Events...> ||
     event_engine<T>)&&event_provider_for<T, Events...>;
} // namespace concepts

} // namespace skizzay::fsm
