#pragma once

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/event_provider.h"
#include "skizzay/fsm/state_provider.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
namespace is_event_context_details_ {
template <typename T>
using add_cref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;
}

template <typename T>
struct is_event_context
    : std::conjunction<is_event_engine<T>, is_event_provider<T>,
                       is_state_provider<T>> {};

namespace concepts {
template <typename T>
concept event_context = is_event_context<T>::value;

template <typename T, typename Event>
concept event_context_for =
    event_context<T> && event<Event> && std::convertible_to<
        is_event_context_details_::add_cref_t<typename T::event_type>,
        is_event_context_details_::add_cref_t<Event>>;
} // namespace concepts

template <typename T, typename Event>
struct is_event_context_for
    : std::bool_constant<concepts::event_context_for<T, Event>> {};

} // namespace skizzay::fsm
