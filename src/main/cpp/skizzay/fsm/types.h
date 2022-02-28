#pragma once

#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

namespace skizzay::fsm {
template <typename T> using event_t = typename T::event_type;

template <typename T> using current_state_t = typename T::current_state_type;

template <typename T> using next_state_t = typename T::next_state_type;

namespace events_list_t_details_ {
template <typename, typename = void> struct impl {};

template <typename T>
struct impl<T, std::void_t<typename T::events_list_type>> {
  using type = typename T::events_list_type;
};

template <typename T>
struct impl<T, std::enable_if_t<is_transition_table<T>::value>> {
  using type = as_container_t<unique_t<map_t<T, event_t>>, events_list>
};
} // namespace events_list_t_details_

template <typename T>
using events_list_t = typename events_list_t_details_::impl<T>::type;

namespace states_list_t_details_ {
template <typename> struct impl;

template <typename T>
requires concepts::states_list<typename T::states_list_type>
struct impl<T> {
  using type = typename T::states_list_type;
};

template <concepts::transition_table T> struct impl<T> {
  using type = as_container_t<
      unique_t<concat_t<map_t<T, next_state_t>, map_t<T, current_state_t>>>,
      states_list>
};
} // namespace states_list_t_details_

template <typename T>
using states_list_t = typename states_list_t_details_::impl<T>::type;

namespace state_t_details_ {
template <typename> struct impl;

template <typename T>
requires concepts::state<typename T::state_type>
struct impl<T> {
  using type = typename T::state_type;
};

template <concepts::states_list T>
requires(1 == length_v<T>) struct impl<T> {
  using type = first_t<T>;
};
} // namespace state_t_details_

template <typename T> using state_t = typename state_t_details_::impl<T>::type;

template <typename T>
using transition_table_t = typename T::transition_table_type;

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

template <typename T>
using current_states_list_t =
    as_container_t<unique_t<map_t<T, current_state_t>>, states_list>;

template <typename T>
using next_states_list_t =
    as_container_t<unique_t<map_t<T, next_state_t>>, states_list>;

template <typename T> using machine_t = typename T::machine_type;
} // namespace skizzay::fsm