#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/optional_reference.h"
#include "skizzay/fsm/query.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {

namespace state_queryable_details_ {

template <typename, typename>
struct has_current_state_template_member_function : std::false_type {};

template <typename T, concepts::state State>
requires requires(T const &tc) {
  { tc.template current_state<State>() }
  noexcept
      ->std::same_as<optional_reference<std::remove_reference_t<State> const>>;
}
struct has_current_state_template_member_function<T, State> : std::true_type {};

template <typename, typename>
struct has_is_template_member_function : std::false_type {};

template <typename T, concepts::state State>
requires requires(T const &tc) {
  { tc.template is<State>() }
  noexcept->concepts::boolean;
}
struct has_is_template_member_function<T, State> : std::true_type {};

template <typename>
struct has_query_template_member_function : std::false_type {};

template <typename T>
requires requires(T const &tc) {
  { tc.query(no_op(true)) };
}
struct has_query_template_member_function<T> : std::true_type {};
} // namespace state_queryable_details_

namespace concepts {
template <typename T>
concept state_queryable = query_result_details_::has_states_list_t<T> &&
    state_queryable_details_::has_query_template_member_function<T>::value &&
    (!empty_v<states_list_t<T>>)&&all_v<
        states_list_t<T>, curry<state_queryable_details_::
                                    has_current_state_template_member_function,
                                T>::template type>
        &&all_v<states_list_t<T>,
                curry<state_queryable_details_::has_is_template_member_function,
                      T>::template type>;
} // namespace concepts

template <typename T>
using is_state_queryable = std::bool_constant<concepts::state_queryable<T>>;

} // namespace skizzay::fsm
