#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
namespace is_event_transition_context_details_ {
template <typename, typename>
struct has_on_transition_template_member_function : std::false_type {};

template <typename T, concepts::transition Transition>
requires requires(T &t, Transition &transition) { t.on_transition(transition); }
struct has_on_transition_template_member_function<T, Transition>
    : std::true_type {};

template <typename, typename>
struct has_get_transitions_template_member_function : std::false_type {};

template <typename T, concepts::state State>
requires requires(T &t, State const &sc) {
  { t.get_transitions(sc) }
  noexcept->concepts::transition_table;
}
struct has_get_transitions_template_member_function<T, State> : std::true_type {
};

template <typename, typename>
struct has_schedule_entry_template_member_function : std::false_type {};

template <typename T, concepts::state State>
requires requires(T &t) {
  { t.template schedule_entry<State>() }
  noexcept;
}
struct has_schedule_entry_template_member_function<T, State> : std::true_type {
};

} // namespace is_event_transition_context_details_

namespace concepts {
template <typename T>
concept event_transition_context =
    std::move_constructible<T> && event_context<T> && requires {
  typename transition_table_t<T>;
} && all_v<current_states_list_t<T>,
           curry<is_event_transition_context_details_::
                     has_get_transitions_template_member_function,
                 T>::template type>
    &&all_v<next_states_list_t<T>,
            curry<is_event_transition_context_details_::
                      has_schedule_entry_template_member_function,
                  T>::template type>
        &&all_v<filter_t<map_t<transition_table_t<T>, std::remove_cvref_t>,
                         curry<std::is_convertible,
                               add_cref_t<event_t<T>>>::template type>,
                curry<is_event_transition_context_details_::
                          has_on_transition_template_member_function,
                      T>::template type>;

template <typename T>
concept final_exit_event_transition_context =
    event_transition_context<T> && std::same_as<event_t<T>, final_exit_event_t>;

template <typename T, typename Event>
concept event_transition_context_for =
    event_transition_context<T> && event<Event> && event_provider_for<T, Event>;
} // namespace concepts

template <typename T>
using is_event_transition_context =
    std::bool_constant<concepts::event_transition_context<T>>;

template <typename T>
using is_final_exit_event_transition_context =
    std::bool_constant<concepts::final_exit_event_transition_context<T>>;

template <typename T, typename Event>
using is_event_transition_context_for =
    std::bool_constant<concepts::event_transition_context_for<T, Event>>;

} // namespace skizzay::fsm
