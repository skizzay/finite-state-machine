#pragma once

#include <skizzay/fsm/state.h>
#include <skizzay/fsm/states_list.h>
#include <skizzay/fsm/type_list.h>

#include <concepts>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {
namespace state_provider_details_ {
template <typename, typename>
struct template_state_member_function : std::false_type {};

template <typename T, typename State>
requires concepts::state<State> && requires(T &t, T const &tc) {
  { t.template state<State>() }
  noexcept->std::same_as<std::add_lvalue_reference_t<State>>;
  { tc.template state<State>() }
  noexcept->std::same_as<std::add_lvalue_reference_t<State const>>;
}
struct template_state_member_function<T, State> : std::true_type {};
} // namespace state_provider_details_

namespace concepts {
template <typename T>
concept state_provider = requires {
  typename states_list_t<T>;
}
&&all_v<states_list_t<T>,
        curry<state_provider_details_::template_state_member_function,
              T>::template type>;

template <typename T, typename State>
concept state_provider_of = state<State> &&
    state_provider_details_::template_state_member_function<T, State>::value;
} // namespace concepts

template <typename T>
using is_state_provider = std::bool_constant<concepts::state_provider<T>>;

} // namespace skizzay::fsm
