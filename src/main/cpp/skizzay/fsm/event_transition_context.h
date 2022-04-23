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
template <typename> struct template_member_function {
  template <typename> struct get_transitions : std::false_type {};
  template <typename> struct on_transition : std::false_type {};
};

template <typename T> template <concepts::state State>
requires requires(T &t, State const &sc) {
  { t.get_transitions(sc) }
  noexcept->concepts::transition_table;
}
struct template_member_function<T>::get_transitions<State> : std::true_type {};

template <typename T> template <concepts::transition Transition>
requires requires(T &t, Transition &transition) { t.on_transition(transition); }
struct template_member_function<T>::on_transition<Transition> : std::true_type {
};

template <typename TransitionTable, template <typename> typename ExtractState>
using extract_state_list_t = as_container_t<
    unique_t<map_t<map_t<TransitionTable, std::remove_cvref_t>, ExtractState>>,
    states_list>;

template <typename TransitionTable>
using extract_current_states_list_t =
    extract_state_list_t<TransitionTable, current_state_t>;
template <typename TransitionTable>
using extract_next_states_list_t =
    extract_state_list_t<TransitionTable, next_state_t>;
} // namespace is_event_transition_context_details_

namespace concepts {
template <typename T>
concept event_transition_context =
    std::move_constructible<T> && event_context<T> && requires {
  typename transition_table_t<T>;
} && all_v<current_states_list_t<T>,
           is_event_transition_context_details_::template_member_function<
               T>::template get_transitions>;

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
