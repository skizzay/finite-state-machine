#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/event_provider.h"
#include "skizzay/fsm/state_provider.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
template <typename T>
struct is_event_context
    : std::conjunction<is_event_engine<T>, is_event_provider<T>,
                       is_state_provider<T>> {};

template <typename> struct is_initial_entry_event_context : std::false_type {};

template <typename T>
requires is_event_context<T>::value struct is_initial_entry_event_context<T>
    : std::is_same<typename T::event_type, initial_entry_event_t> {
};

namespace concepts {
template <typename T>
concept event_context = is_event_context<T>::value;

template <typename T>
concept initial_entry_event_context = is_initial_entry_event_context<T>::value;
} // namespace concepts

} // namespace skizzay::fsm
