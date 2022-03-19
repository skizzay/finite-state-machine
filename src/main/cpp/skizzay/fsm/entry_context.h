#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/states_list.h"

#include <type_traits>

namespace skizzay::fsm {

namespace is_entry_context_details_ {
template <typename T> struct template_member_function {
  template <typename> struct is_scheduled : std::false_type {};
};

template <typename T> template <typename State>
requires concepts::state_in<State, typename T::states_list_type> &&
    requires(T const &tc) {
  { tc.template is_scheduled<State>() }
  noexcept->concepts::boolean;
}
struct template_member_function<T>::is_scheduled<State> : std::true_type {};
} // namespace is_entry_context_details_

template <typename> struct is_entry_context : std::false_type {};

template <typename T>
requires concepts::event_context<T> &&
    all_v<typename T::states_list_type,
          is_entry_context_details_::template_member_function<
              T>::template is_scheduled> struct is_entry_context<T>
    : std::true_type {
};

namespace concepts {
template <typename T>
concept entry_context = is_entry_context<T>::value;

template <typename T>
concept initial_entry_context = entry_context<T> &&
    std::same_as<initial_entry_event_t, typename T::event_type>;
} // namespace concepts

template <typename T>
struct is_initial_entry_context
    : std::bool_constant<concepts::initial_entry_context<T>> {};

} // namespace skizzay::fsm
