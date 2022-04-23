#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"

#include <type_traits>

namespace skizzay::fsm {

namespace is_entry_context_details_ {
template <typename T> struct template_member_function {
  template <typename> struct is_scheduled : std::false_type {};
};

template <typename T> template <typename State>
requires concepts::state_in<State, next_states_list_t<T>> &&
    requires(T const &tc) {
  { tc.template is_scheduled<State>() }
  noexcept->concepts::boolean;
}
struct template_member_function<T>::is_scheduled<State> : std::true_type {};
} // namespace is_entry_context_details_

namespace concepts {
template <typename T>
concept entry_context = event_context<T> && requires {
  typename next_states_list_t<T>;
} && all_v<next_states_list_t<T>,
           is_entry_context_details_::template_member_function<
               T>::template is_scheduled>;

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
