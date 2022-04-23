#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/event_provider.h"
#include "skizzay/fsm/state_provider.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {

namespace concepts {
template <typename T>
concept event_context =
    event_engine<T> && event_provider<T> && state_provider<T>;

template <typename T>
concept initial_entry_event_context =
    event_context<T> && std::same_as<event_t<T>, initial_entry_event_t>;

template <typename T, typename Event>
concept event_context_for = event_context<T> && event_provider_for<T, Event>;
} // namespace concepts

template <typename T, typename U>
struct is_event_context_for
    : std::bool_constant<concepts::event_context_for<T, U>> {};
} // namespace skizzay::fsm
