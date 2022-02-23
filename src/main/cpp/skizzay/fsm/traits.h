#pragma once

#include <skizzay/fsm/type_list.h>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {

struct initial_entry_event_t;
struct final_exit_event_t;


template <typename> struct is_type_list : std::false_type {};

template <template <typename...> typename Template, typename... Ts>
struct is_type_list<Template<Ts...>> : std::true_type {};

template <typename T, template <typename...> typename Template>
class is_template {
  static void test(...);
  template <typename... Args>
  static Template<Args...> test(Template<Args...> *const);

public:
  using type =
      decltype(is_template<T, Template>::test(static_cast<T *>(nullptr)));
  using value_t = std::is_same<T, type>;
  static constexpr bool value = value_t::value;
};

template <typename T> struct is_integer_sequence : std::false_type {};
template <std::integral Integral, Integral... Is>
struct is_integer_sequence<std::integer_sequence<Integral, Is...>>
    : std::true_type {};

template <typename, typename = void>
struct has_states_list_type : std::false_type {};

template <typename T>
struct has_states_list_type<T, std::void_t<typename T::states_list_type>>
    : is_type_list<typename T::states_list_type> {};

template <typename, typename = void>
struct has_events_list_type : std::false_type {};

template <typename T>
struct has_events_list_type<T, std::void_t<typename T::events_list_type>>
    : is_type_list<typename T::events_list_type> {};

template <typename T> using is_transition_coordinator = std::true_type;

template <typename T> using is_entry_coordinator = std::true_type;

namespace containers_containing_state_details_ {
template <typename StateContainersList, typename State> class impl {
  template <typename StateContainer>
  using test = contains<typename StateContainer::states_list_type, State>;

public:
  using type = filter_t<StateContainersList, test>;
};

template <typename StateContainersList, typename TransitionTable,
          typename Event>
class current_state_impl {
  template <typename StateContainer, typename Transition>
  using has_current_state = contains<typename StateContainer::states_list_type,
                                     typename Transition::current_state_type>;

  template <typename StateContainer> struct is_compatible_transition {
    template <typename Transition>
    using test = std::conjunction<
        has_current_state<StateContainer, Transition>,
        std::is_convertible<
            std::remove_cvref_t<typename Transition::event_type> const &,
            Event const &>>;
  };

  template <typename StateContainer>
  using has_applicable_transition = std::negation<
      empty<filter_t<TransitionTable,
                     is_compatible_transition<StateContainer>::template test>>>;

public:
  using type =
      filter_t<StateContainersList,
               current_state_impl<StateContainersList, TransitionTable,
                                  Event>::template has_applicable_transition>;
};
} // namespace containers_containing_state_details_

template <typename StateContainersList, typename State>
using containers_containing_state_t =
    typename containers_containing_state_details_::impl<
        std::enable_if_t<is_type_list<StateContainersList>::value,
                         StateContainersList>,
        std::enable_if_t<is_state<State>::value, State>>::type;

template <typename StateContainersList, typename State>
using state_container_for_t =
    front_t<containers_containing_state_t<StateContainersList, State>>;

template <typename StateContainersList, typename TransitionTable,
          typename Event>
using containers_of_current_state_t =
    typename containers_containing_state_details_::current_state_impl<
        StateContainersList, TransitionTable, Event>::type;

template <typename T>
using current_states_list_t =
    as_container_t<unique_t<map_t<T, current_state_t>>, states_list>;

template <typename T>
using next_states_list_t =
    as_container_t<unique_t<map_t<T, next_state_t>>, states_list>;

} // namespace skizzay::fsm
