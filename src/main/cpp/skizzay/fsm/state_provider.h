#pragma once

#include <skizzay/fsm/state.h>
#include <skizzay/fsm/states_list.h>
#include <skizzay/fsm/type_list.h>

#include <concepts>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_state_provider : std::false_type {};

namespace state_provider_details_ {
template <typename T> struct template_member_function {
  template <typename> struct state : std::false_type {};
};

template <typename T> template <typename State>
requires concepts::state<State> && requires(T &t, T const &tc) {
  { t.template state<State>() }
  noexcept->std::same_as<std::add_lvalue_reference_t<State>>;
  { tc.template state<State>() }
  noexcept->std::same_as<std::add_lvalue_reference_t<State const>>;
}
struct template_member_function<T>::state<State> : std::true_type {};
} // namespace state_provider_details_

template <typename T, typename State>
struct is_state_provider_of : state_provider_details_::template_member_function<
                                  T>::template state<State> {};

template <typename T>
requires concepts::states_list<typename T::states_list_type> &&
    all_v<typename T::states_list_type,
          state_provider_details_::template_member_function<
              T>::template state> struct is_state_provider<T> : std::true_type {
};

namespace concepts {
template <typename T>
concept state_provider = is_state_provider<T>::value;

template <typename T, typename State>
concept state_provider_of = is_state_provider_of<T, State>::value;
} // namespace concepts

} // namespace skizzay::fsm
