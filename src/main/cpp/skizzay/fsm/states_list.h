#pragma once

#include <skizzay/fsm/state.h>
#include <skizzay/fsm/type_list.h>

#include <type_traits>

namespace skizzay::fsm {
template <typename> struct is_states_list : std::false_type {};

template <template <typename...> typename Template, concepts::state... States>
struct is_states_list<Template<States...>> : std::true_type {};

namespace concepts {
template <typename T>
concept states_list = is_states_list<T>::value;

template <typename State, typename StatesList>
concept state_in =
    state<State> && states_list<StatesList> && contains_v<StatesList, State>;
} // namespace concepts

template <typename State, typename StatesList>
struct is_state_in : std::bool_constant<concepts::state_in<State, StatesList>> {
};

template <typename> struct basic_states_list_t;

template <typename T>
using states_list_t = typename basic_states_list_t<T>::type;

template <typename T>
requires concepts::states_list<typename T::states_list_type>
struct basic_states_list_t<T> {
  using type = typename T::states_list_type;
};

template <concepts::states_list T>
requires(1 == length_v<T>) struct basic_state_t<T> {
  using type = front_t<T>;
};

template <typename> struct basic_current_states_list_t;

namespace current_states_list_t_details_ {
template <typename> struct impl;

template <typename T>
requires concepts::states_list<typename basic_current_states_list_t<T>::type>
struct impl<T> {
  using type = typename basic_current_states_list_t<T>::type;
};
} // namespace current_states_list_t_details_

template <typename T>
using current_states_list_t =
    typename current_states_list_t_details_::impl<T>::type;

template <typename T>
requires concepts::states_list<typename T::current_states_list_type>
struct basic_current_states_list_t<T> {
  using type = typename T::current_states_list_type;
};

template <typename> struct basic_next_states_list_t;

namespace next_states_list_t_details_ {
template <typename> struct impl;

template <typename T>
requires concepts::states_list<typename basic_next_states_list_t<T>::type>
struct impl<T> {
  using type = typename basic_next_states_list_t<T>::type;
};
} // namespace next_states_list_t_details_

template <typename T>
using next_states_list_t =
    typename next_states_list_t_details_::impl<T>::type;

template <typename T>
requires concepts::states_list<typename T::next_states_list_type>
struct basic_next_states_list_t<T> {
  using type = typename T::next_states_list_type;
};

template <concepts::state... States> struct states_list {};
} // namespace skizzay::fsm
