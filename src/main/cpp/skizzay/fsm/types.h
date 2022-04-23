#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/state_containers_list.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {

// namespace states_list_t_details_ {
// template <typename> struct impl;

// template <typename T>
// requires concepts::states_list<typename T::states_list_type>
// struct impl<T> {
//   using type = typename T::states_list_type;
// };

// template <concepts::transition_table T> struct impl<T> {
//   using type = as_container_t<
//       unique_t<concat_t<map_t<map_t<T, std::remove_cvref_t>, next_state_t>,
//                         map_t<map_t<T, std::remove_cvref_t>,
//                         current_state_t>>>,
//       states_list>;
// };

// template <template <typename...> typename Template,
//           concepts::state_container... StateContainers>
// struct impl<Template<StateContainers...>> {
//   using type = concat_t<
//       as_container_t<typename impl<StateContainers>::type, states_list>...>;
// };
// } // namespace states_list_t_details_

// template <typename T>
// using states_list_t = typename states_list_t_details_::impl<T>::type;

// namespace state_t_details_ {

// template <typename T>
// concept has_state_type = concepts::state<typename T::state_type>;

// template <typename> struct impl;

// template <typename T>
// requires has_state_type<T>
// struct impl<T> {
//   using type = typename T::state_type;
// };

// template <concepts::states_list T>
// requires(!has_state_type<T>) && (1 == length_v<T>)struct impl<T> {
//   using type = front_t<T>;
// };

// template <typename T>
// requires(!has_state_type<T>) &&
//     (!concepts::states_list<T>)&&concepts::states_list<
//         states_list_t<T>> struct impl<T> : impl<states_list_t<T>> {
// };
// } // namespace state_t_details_

// template <typename T> using state_t = typename
// state_t_details_::impl<T>::type;

template <typename T> using machine_t = typename T::machine_type;
} // namespace skizzay::fsm