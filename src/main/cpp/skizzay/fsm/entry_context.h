#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state_schedule.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"

#include <type_traits>

namespace skizzay::fsm {

namespace concepts {
template <typename T>
concept entry_context = event_context<T> && state_schedule<T>;

template <typename T>
concept initial_entry_context =
    entry_context<T> && std::same_as<event_t<T>, initial_entry_event_t>;
} // namespace concepts

template <typename T>
using is_entry_context = std::bool_constant<concepts::entry_context<T>>;

template <typename T>
using is_initial_entry_context =
    std::bool_constant<concepts::initial_entry_context<T>>;

} // namespace skizzay::fsm
