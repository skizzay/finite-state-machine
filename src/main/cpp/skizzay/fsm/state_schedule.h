#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {
namespace state_schedule_details_ {

template <typename, typename>
struct has_is_scheduled_template_member_function : std::false_type {};
// template <typename, typename>
// struct has_schedule_entry_template_member_function : std::false_type {};

template <typename T, concepts::state State>
requires requires(T const &tc) {
  { tc.template is_scheduled<State>() }
  noexcept->concepts::boolean;
}
struct has_is_scheduled_template_member_function<T, State> : std::true_type {};

// template <typename T, concepts::state State>
// requires requires(T &t) {
//   { t.template schedule_entry<State>() }
//   noexcept;
// }
// struct has_schedule_entry_template_member_function<T, State> : std::true_type {
// };
} // namespace state_scheduler_details_

namespace concepts {
template <typename T>
concept state_schedule = requires {
  typename next_states_list_t<T>;
}
&&all_v<
    next_states_list_t<T>,
    curry<state_schedule_details_::has_is_scheduled_template_member_function,
          T>::template type>;
} // namespace concepts
template <typename T>
struct is_state_schedule : std::bool_constant<concepts::state_schedule<T>> {};
} // namespace skizzay::fsm