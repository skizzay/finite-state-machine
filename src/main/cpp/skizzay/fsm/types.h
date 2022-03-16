#pragma once

#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <type_traits>

namespace skizzay::fsm {
struct nonesuch final {
  nonesuch() = delete;
  nonesuch(nonesuch const &) = delete;
  ~nonesuch() = delete;

  void operator=(nonesuch const &) = delete;
};

namespace is_detected_details_ {
template <typename Default, typename AlwaysVoid,
          template <typename...> typename, typename...>
struct impl {
  using type = Default;
  using value_type = std::false_type;
};

template <typename Default, template <typename...> typename Template,
          typename... Args>
struct impl<Default, std::void_t<Template<Args...>>, Template, Args...> {
  using type = Template<Args...>;
  using value_type = std::true_type;
};
} // namespace is_detected_details_

template <template <typename...> typename Template, typename... Ts>
struct is_detected
    : is_detected_details_::impl<nonesuch, void, Template, Ts...>::value_type {
};

template <template <typename...> typename Template, typename... Ts>
constexpr inline bool is_detected_v = is_detected<Template, Ts...>::value;

template <template <typename...> typename Template, typename... Ts>
struct detected {
  using type = typename is_detected_details_::impl<nonesuch, void, Template,
                                                   Ts...>::type;
};

template <template <typename...> typename Template, typename... Ts>
using detected_t = typename detected<Template, Ts...>::type;

template <typename Default, template <typename...> typename Template,
          typename... Ts>
struct detected_or {
  using type =
      typename is_detected_details_::impl<Default, void, Template, Ts...>::type;
};

template <typename Default, template <typename...> typename Template,
          typename... Ts>
using detected_or_t = typename detected_or<Default, Template, Ts...>::type;

template <typename T> using event_t = typename T::event_type;

template <typename T> using current_state_t = typename T::current_state_type;

template <typename T> using next_state_t = typename T::next_state_type;

namespace events_list_t_details_ {
template <typename> struct impl {};

template <typename T>
requires concepts::events_list<typename T::events_list_type>
struct impl<T> {
  using type = typename T::events_list_type;
};

template <typename T>
requires concepts::transition_table<T>
struct impl<T> {
  using type =
      as_container_t<unique_t<map_t<map_t<T, std::remove_cvref_t>, event_t>>,
                     events_list>;
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
      unique_t<concat_t<map_t<map_t<T, std::remove_cvref_t>, next_state_t>,
                        map_t<map_t<T, std::remove_cvref_t>, current_state_t>>>,
      states_list>;
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
  using type = front_t<T>;
};

template <typename T>
requires concepts::states_list<states_list_t<T>>
struct impl<T> : impl<states_list_t<T>> {
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

namespace extract_states_list_type_details_ {
template <typename, template <typename> typename> struct impl {};

template <concepts::transition_table TransitionTable,
          template <typename> typename ExtractState>
struct impl<TransitionTable, ExtractState> {
  using type =
      as_container_t<unique_t<map_t<map_t<TransitionTable, std::remove_cvref_t>,
                                    ExtractState>>,
                     states_list>;
};

template <typename T, template <typename> typename ExtractState>
requires concepts::transition_table<transition_table_t<T>>
struct impl<T, ExtractState> : impl<transition_table_t<T>, ExtractState> {
};
} // namespace extract_states_list_type_details_

template <typename T>
using current_states_list_t =
    typename extract_states_list_type_details_::impl<T, current_state_t>::type;

template <typename T>
using next_states_list_t =
    typename extract_states_list_type_details_::impl<T, next_state_t>::type;

template <typename T> using machine_t = typename T::machine_type;
} // namespace skizzay::fsm