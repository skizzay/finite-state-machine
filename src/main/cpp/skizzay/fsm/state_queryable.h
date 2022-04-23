#pragma once

#include <skizzay/fsm/boolean.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/state.h>
#include <skizzay/fsm/states_list.h>
#include <skizzay/fsm/type_list.h>

#include <type_traits>

namespace skizzay::fsm {
namespace state_queryable_details_ {
template <typename T> struct template_member_function {
  template <typename> struct current_state : std::false_type {};
  template <typename> struct is : std::false_type {};
};

template <typename T> template <typename State>
requires concepts::state<State> && requires(T const &tc) {
  { tc.template current_state<State>() }
  noexcept
      ->std::same_as<optional_reference<std::remove_reference_t<State> const>>;
}
struct template_member_function<T>::current_state<State> : std::true_type {};

template <typename T> template <typename State>
requires concepts::state<State> && requires(T const &tc) {
  { tc.template is<State>() }
  noexcept->concepts::boolean;
}
struct template_member_function<T>::is<State> : std::true_type {};
} // namespace state_queryable_details_

namespace concepts {
template <typename T>
concept state_queryable = requires {
  typename states_list_t<T>;
}
&&(!empty_v<states_list_t<T>>)&&all_v<
    states_list_t<T>, state_queryable_details_::template_member_function<
                          T>::template current_state>
    &&all_v<states_list_t<T>,
            state_queryable_details_::template_member_function<T>::template is>;
} // namespace concepts

template <typename T>
using is_state_queryable = std::bool_constant<concepts::state_queryable<T>>;

} // namespace skizzay::fsm
