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
template <typename> struct is_event_transition_context : std::false_type {};

template <typename>
struct is_final_exit_event_transition_context : std::false_type {};

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

template <typename T>
using extract_current_state_t = typename T::current_state_type;

template <typename T> using extract_next_state_t = typename T::next_state_type;

template <typename TransitionTable>
using extract_current_states_list_t =
    extract_state_list_t<TransitionTable, extract_current_state_t>;
template <typename TransitionTable>
using extract_next_states_list_t =
    extract_state_list_t<TransitionTable, extract_next_state_t>;
} // namespace is_event_transition_context_details_

template <typename T>
requires std::move_constructible<T> && concepts::event_context<T> &&
    concepts::transition_table<typename T::transition_table_type> &&
    all_v<is_event_transition_context_details_::extract_current_states_list_t<
              typename T::transition_table_type>,
          is_event_transition_context_details_::template_member_function<T>::
              template get_transitions> struct is_event_transition_context<T>
    : std::true_type {
};

template <typename T>
requires is_event_transition_context<
    T>::value struct is_final_exit_event_transition_context<T>
    : std::is_same<typename T::event_type, final_exit_event_t> {
};

namespace concepts {
template <typename T>
concept event_transition_context = is_event_transition_context<T>::value;

template <typename T>
concept final_exit_event_transition_context =
    is_final_exit_event_transition_context<T>::value;
} // namespace concepts

} // namespace skizzay::fsm
