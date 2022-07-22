#pragma once

#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/state_schedule.h"

#include <type_traits>

namespace skizzay::fsm {

namespace concepts {
template <typename T>
concept entry_context = state_provider<T> && state_schedule<T>;
} // namespace concepts

template <typename T>
using is_entry_context = std::bool_constant<concepts::entry_context<T>>;

} // namespace skizzay::fsm
