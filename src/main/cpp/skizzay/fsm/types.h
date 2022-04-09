#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/state_containers_list.h"
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

namespace event_t_details_ {
template <typename T> struct impl {
  using type = std::enable_if_t<concepts::event<T>, T>;
};

template <typename T>
requires concepts::event<typename T::event_type>
struct impl<T> : impl<typename T::event_type> {
};
} // namespace event_t_details_

template <typename T> using event_t = typename event_t_details_::impl<T>::type;

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

template <typename T>
concept has_state_type = concepts::state<typename T::state_type>;

template <typename> struct impl;

template <typename T>
requires has_state_type<T>
struct impl<T> {
  using type = typename T::state_type;
};

template <concepts::states_list T>
requires(!has_state_type<T>) && (1 == length_v<T>)struct impl<T> {
  using type = front_t<T>;
};

template <typename T>
requires(!has_state_type<T>) &&
    (!concepts::states_list<T>)&&concepts::states_list<
        states_list_t<T>> struct impl<T> : impl<states_list_t<T>> {
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

template <typename T>
struct current_states_list_impl : impl<T, current_state_t> {};

template <typename T>
requires concepts::states_list<typename T::current_states_list_type>
struct current_states_list_impl<T> {
  using type = typename T::current_states_list_type;
};

template <typename T> struct next_states_list_impl : impl<T, next_state_t> {};

template <typename T>
requires concepts::states_list<typename T::next_states_list_type>
struct next_states_list_impl<T> {
  using type = typename T::next_states_list_type;
};
} // namespace extract_states_list_type_details_

template <typename T>
using current_states_list_t =
    typename extract_states_list_type_details_::current_states_list_impl<
        T>::type;

template <typename T>
using next_states_list_t =
    typename extract_states_list_type_details_::next_states_list_impl<T>::type;

namespace state_containers_for_details_ {
template <typename, typename> struct impl {};

template <concepts::states_list StatesList> struct has_state_in {
  template <concepts::state_container StateContainer>
  using test = contains_any<states_list_t<StateContainer>, StatesList>;
};

template <concepts::state_containers_list StateContainersList,
          concepts::states_list StatesList>
struct impl<StateContainersList, StatesList> {
  using type =
      filter_t<StateContainersList, has_state_in<StatesList>::template test>;
};
} // namespace state_containers_for_details_

template <typename StateContainersList, typename StatesList>
using state_containers_for_t =
    typename state_containers_for_details_::impl<StateContainersList,
                                                 StatesList>::type;

namespace is_event_context_for_details_ {
template <template <typename> typename Predicate, typename Candidate,
          typename U>
struct impl : std::false_type {};

template <template <typename> typename Predicate, typename Candidate,
          typename U>
requires Predicate<Candidate>::value struct impl<Predicate, Candidate, U>
    : std::is_convertible<add_cref_t<event_t<Candidate>>,
                          add_cref_t<event_t<U>>> {
};
} // namespace is_event_context_for_details_

template <typename T, typename... Us>
struct is_event_context_for : std::conjunction<is_event_context_for<T, Us>...> {
};

template <typename EventContext, typename U>
struct is_event_context_for<EventContext, U>
    : is_event_context_for_details_::impl<is_event_context, EventContext, U> {};

template <typename T, typename... Us>
struct is_event_transition_context_for
    : std::conjunction<is_event_transition_context_for<T, Us>...> {};

template <typename EventTransitionContext, typename U>
struct is_event_transition_context_for<EventTransitionContext, U>
    : is_event_context_for_details_::impl<is_event_transition_context,
                                          EventTransitionContext, U> {};

namespace concepts {
template <typename T, typename U>
concept event_context_for = is_event_context_for<T, U>::value;

template <typename T, typename U>
concept event_transition_context_for =
    is_event_transition_context_for<T, U>::value;
} // namespace concepts

template <typename T> using machine_t = typename T::machine_type;
} // namespace skizzay::fsm