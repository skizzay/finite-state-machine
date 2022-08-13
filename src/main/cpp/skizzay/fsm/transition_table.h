#pragma once

#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/states_list.h"
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

template <typename T>
concept root_transition_table =
    transition_table<T> && std::same_as<T, map_t<T, std::remove_cvref_t>>;

template <typename T>
concept event_node_transition_table = transition_table<T> &&
    std::same_as<T, map_t<T, std::add_lvalue_reference_t>>;

template <typename Transition, typename TransitionTable>
concept transition_in = transition<std::remove_cvref_t<Transition>> &&
    transition_table<TransitionTable> &&
    contains_v<map_t<TransitionTable, std::remove_cvref_t>,
               std::remove_cvref_t<Transition>>;

template <typename Transition, typename TransitionTable>
concept self_transition_in =
    self_transition<Transition> && transition_in<Transition, TransitionTable>;
} // namespace concepts

template <typename Transition, typename TransitionTable>
using is_transition_in =
    std::bool_constant<concepts::transition_in<Transition, TransitionTable>>;

namespace transition_table_t_details_ {
template <typename> struct impl;

template <typename T>
requires concepts::transition_table<typename T::transition_table_type>
struct impl<T> {
  using type = typename T::transition_table_type;
};
} // namespace transition_table_t_details_

template <typename T>
using transition_table_t = typename transition_table_t_details_::impl<T>::type;

namespace transition_at_details_ {
template <std::size_t, typename> struct impl {};

template <std::size_t I, concepts::transition_table TransitionTable>
struct impl<I, TransitionTable> {
  using type = element_at_t<I, TransitionTable>;
};

template <std::size_t I, typename T>
requires concepts::transition_table<transition_table_t<T>>
struct impl<I, T> {
  using type = typename impl<I, transition_table_t<T>>::type;
};
} // namespace transition_at_details_

template <std::size_t I, typename T>
using transition_at_t = typename transition_at_details_::impl<I, T>::type;

namespace extract_transition_t_details_ {
template <concepts::transition_table TransitionTable,
          template <typename> typename ExtractType,
          template <typename...> typename List>
using impl = as_container_t<
    unique_t<map_t<map_t<TransitionTable, std::remove_cvref_t>, ExtractType>>,
    List>;
}

template <concepts::transition_table TransitionTable>
struct basic_current_states_list_t<TransitionTable> {
  using type =
      extract_transition_t_details_::impl<TransitionTable, current_state_t,
                                          states_list>;
};

template <typename T>
requires requires { typename transition_table_t<T>; }
struct basic_current_states_list_t<T>
    : basic_current_states_list_t<transition_table_t<T>> {};

template <concepts::transition_table TransitionTable>
struct basic_next_states_list_t<TransitionTable> {
  using type = extract_transition_t_details_::impl<TransitionTable,
                                                   next_state_t, states_list>;
};

template <typename T>
requires requires { typename transition_table_t<T>; }
struct basic_next_states_list_t<T>
    : basic_next_states_list_t<transition_table_t<T>> {};

namespace transition_table_details_ {
template <concepts::event Event, concepts::state State,
          concepts::transition Transition>
struct has_current_state
    : std::conjunction<std::is_same<State, current_state_t<Transition>>,
                       is_event_in<Event, Transition>> {};

template <typename State, typename TransitionTable, typename Event>
concept current_state_in =
    concepts::state<State> && concepts::transition_table<TransitionTable> &&
    concepts::event_in<Event, TransitionTable> &&
    any_v<TransitionTable,
          curry<has_current_state, Event, State>::template type>;
} // namespace transition_table_details_

template <concepts::event Event, concepts::transition_table TransitionTable>
struct is_event_in<Event, TransitionTable>
    : any<map_t<TransitionTable, std::remove_cvref_t>,
          curry<is_event_in, Event>::template type> {};

template <concepts::event Event, typename T>
requires requires { typename transition_table_t<T>; }
struct is_event_in<Event, T> : is_event_in<Event, transition_table_t<T>> {};

template <concepts::transition_table T> struct basic_events_list_t<T> {
  using type = extract_transition_t_details_::impl<T, event_t, events_list>;
};

template <typename T>
requires requires {
  typename transition_table_t<T>;
} &&(!requires { typename T::events_list_type; }) struct basic_events_list_t<T>
    : basic_events_list_t<transition_table_t<T>> {
};

template <
    concepts::transition_table TransitionTable,
    concepts::event_in<TransitionTable> Event,
    transition_table_details_::current_state_in<TransitionTable, Event> State>
constexpr concepts::transition_table auto
get_transition_table_for_current_state(TransitionTable &transition_table,
                                       Event const &, State const &) noexcept {
  using candidate_transitions_list_type = as_container_t<
      filter_t<TransitionTable,
               curry<transition_table_details_::has_current_state, Event,
                     State>::template type>,
      simple_type_list>;

  return []<concepts::transition... Transitions>(
      TransitionTable & transition_table,
      simple_type_list<Transitions...>) noexcept {
    return std::tie(std::get<Transitions>(transition_table)...);
  }
  (transition_table, candidate_transitions_list_type{});
}

template <concepts::transition_table TransitionTable, concepts::event Event,
          concepts::state State>
requires(!transition_table_details_::current_state_in<State, TransitionTable,
                                                      Event>) constexpr std::
    tuple<> get_transition_table_for_current_state(TransitionTable &,
                                                   Event const &,
                                                   State const &) noexcept {
  return {};
}

} // namespace skizzay::fsm
