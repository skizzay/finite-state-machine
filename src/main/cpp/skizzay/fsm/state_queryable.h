#pragma once

#include <skizzay/fsm/boolean.h>
#include <skizzay/fsm/optional_reference.h>
#include <skizzay/fsm/state.h>
#include <skizzay/fsm/states_list.h>
#include <skizzay/fsm/type_list.h>

#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_state_queryable : std::false_type {};

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

template <typename T>
// TODO: Check for visitation
requires concepts::states_list<typename T::states_list_type> &&
    (0 < length_v<typename T::states_list_type>)&&all_v<
        typename T::states_list_type,
        state_queryable_details_::template_member_function<
            T>::template current_state>
        &&all_v<typename T::states_list_type,
                state_queryable_details_::template_member_function<
                    T>::template is> struct is_state_queryable<T>
    : std::true_type {};

namespace concepts {
template <typename T>
concept state_queryable = is_state_queryable<T>::value;
}

} // namespace skizzay::fsm
