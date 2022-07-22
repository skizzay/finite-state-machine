#pragma once

#include <skizzay/fsm/state.h>
#include <skizzay/fsm/type_list.h>

#include <type_traits>

namespace skizzay::fsm {

template <concepts::state... States> struct states_list {};

template <typename> struct is_states_list : std::false_type {};

template <concepts::state... States>
struct is_states_list<states_list<States...>> : std::true_type {};

template <typename, typename> struct is_state_in : std::false_type {};

namespace concepts {
template <typename T>
concept states_list = is_states_list<T>::value;

template <typename State, typename StatesList>
concept state_in = is_state_in<State, StatesList>::value;
} // namespace concepts

template <concepts::state State, concepts::states_list StatesList>
struct is_state_in<State, StatesList>
    : contains<map_t<StatesList, std::remove_cvref_t>,
               std::remove_cvref_t<State>> {};

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
requires requires {
  typename T::current_states_list_type;
} && concepts::states_list<typename T::current_states_list_type>
struct impl<T> {
  using type = typename T::current_states_list_type;
};

template <typename T>
requires concepts::states_list<typename basic_current_states_list_t<T>::type> &&
    (!requires { typename T::current_states_list_type; }) struct impl<T> {
  using type = typename basic_current_states_list_t<T>::type;
};

template <typename T>
requires std::is_reference_v<T>
struct impl<T> : impl<std::remove_reference_t<T>> {
};
} // namespace current_states_list_t_details_

template <typename T>
using current_states_list_t =
    typename current_states_list_t_details_::impl<T>::type;

template <typename> struct basic_next_states_list_t;

namespace next_states_list_t_details_ {
template <typename> struct impl;

template <typename T>
requires requires {
  typename T::next_states_list_type;
} && concepts::states_list<typename T::next_states_list_type>
struct impl<T> {
  using type = typename T::next_states_list_type;
};

template <typename T>
requires concepts::states_list<typename basic_next_states_list_t<T>::type> &&
    (!requires { typename T::next_states_list_type; }) struct impl<T> {
  using type = typename basic_next_states_list_t<T>::type;
};

template <typename T> struct impl<T &> : impl<std::remove_cvref_t<T>> {};
} // namespace next_states_list_t_details_

template <typename T>
using next_states_list_t = typename next_states_list_t_details_::impl<T>::type;

template <typename T>
requires(!requires { typename T::states_list_type; }) && requires {
  typename current_states_list_t<T>;
  typename next_states_list_t<T>;
}
struct basic_states_list_t<T>
    : unique<concat_t<current_states_list_t<T>, next_states_list_t<T>>> {};

template <concepts::state State, typename T>
requires std::is_reference_v<T>
struct is_state_in<State, T> : is_state_in<State, std::remove_reference_t<T>> {
};

template <concepts::state State, typename T>
requires requires { typename states_list_t<T>; }
struct is_state_in<State, T>
    : contains<map_t<states_list_t<T>, std::remove_cvref_t>,
               std::remove_cvref_t<State>> {};

template <concepts::state State, typename T>
requires std::is_reference_v<T>
struct is_current_state_in<State, T>
    : is_current_state_in<State, std::remove_reference_t<T>> {
};

template <concepts::state State, typename T>
requires requires { typename current_states_list_t<T>; }
struct is_current_state_in<State, T>
    : contains<std::remove_cvref_t<State>, current_states_list_t<T>> {};

template <concepts::state State, typename T>
requires std::is_reference_v<T>
struct is_next_state_in<State, T>
    : is_next_state_in<State, std::remove_reference_t<T>> {
};

template <concepts::state State, typename T>
requires requires { typename next_states_list_t<T>; }
struct is_next_state_in<State, T>
    : contains<std::remove_cvref_t<State>, next_states_list_t<T>> {};
} // namespace skizzay::fsm
