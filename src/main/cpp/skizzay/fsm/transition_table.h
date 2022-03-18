#pragma once

#include "skizzay/fsm/traits.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/type_list.h"

namespace skizzay::fsm {
template <typename> struct is_transition_table : std::false_type {};

template <typename... Ts>
struct is_transition_table<std::tuple<Ts...>>
    : all<std::tuple<std::remove_cvref_t<Ts>...>, is_transition> {};

namespace concepts {
template <typename T>
concept transition_table = is_transition_table<T>::value;

template <typename Transition, typename TransitionTable>
concept transition_in = transition<std::remove_cvref_t<Transition>> &&
    transition_table<TransitionTable> &&
    contains_v<TransitionTable, Transition>;
} // namespace concepts

template <typename Transition, typename TransitionTable>
struct is_transition_in
    : std::bool_constant<concepts::transition_in<Transition, TransitionTable>> {
};

namespace transition_table_details_ {
template <concepts::event Event, concepts::state State>
struct has_current_state {
  template <concepts::transition Transition>
  using test = std::conjunction<
      std::is_same<State, typename Transition::current_state_type>,
      std::is_convertible<Event const &,
                          typename Transition::event_type const &>>;
};
} // namespace transition_table_details_

template <concepts::transition_table TransitionTable, concepts::state State,
          concepts::event Event>
constexpr concepts::transition_table auto
get_transition_table_for_current_state(TransitionTable &transition_table,
                                       State const &, Event const &) noexcept {
  using candidate_transitions_list_type = as_container_t<
      filter_t<TransitionTable, transition_table_details_::has_current_state<
                                    Event, State>::template test>,
      simple_type_list>;

  if constexpr (empty_v<candidate_transitions_list_type>) {
    return std::tuple{};
  } else {
    return []<concepts::transition... Transitions>(
        TransitionTable & transition_table,
        simple_type_list<Transitions...>) noexcept {
      return std::tie(std::get<Transitions>(transition_table)...);
    }
    (transition_table, candidate_transitions_list_type{});
  }
}

} // namespace skizzay::fsm
