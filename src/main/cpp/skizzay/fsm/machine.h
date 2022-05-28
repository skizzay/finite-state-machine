#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event_handler.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_queryable.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {

namespace concepts {
template <typename T>
concept machine = std::destructible<T> && std::copy_constructible<T> &&
    state_queryable<T> && event_handler<T> && requires(T &t, T const &tc) {
  typename transition_table_t<T>;
  {t.start()};
  {t.stop()};
  { tc.is_running() }
  noexcept->boolean;
  { tc.is_stopped() }
  noexcept->boolean;
} &&(!empty_v<transition_table_t<T>>);
} // namespace concepts

template <typename T>
using is_machine = std::bool_constant<concepts::machine<T>>;

} // namespace skizzay::fsm
