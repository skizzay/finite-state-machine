#pragma once

#include <skizzay/fsm/type_list.h>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {

struct initial_entry_event_t;
struct final_exit_event_t;

namespace details_ {
struct dummy_event {};

struct dummy_state {};

struct dummy_transition {
  using current_state_type = dummy_state;
  using next_state_type = dummy_state;
  using event_type = dummy_event;

  template <typename Machine>
  bool accepts(current_state_type const &, Machine const &,
               event_type const &) const noexcept;
};

struct dummy_action_transition : dummy_transition {
  void on_triggered(event_type const &);
};

template <typename StatesListType, typename EventsListType,
          typename TransitionTableType = std::tuple<>>
struct dummy_machine {
  using states_list_type = StatesListType;
  using events_list_type = EventsListType;
  using transition_table_type = TransitionTableType;
  template <typename...> constexpr bool is() const noexcept { return false; }
  constexpr bool on(auto const &) noexcept { return true; }
  constexpr void start() noexcept {}
  constexpr void stop() noexcept {}
};

struct dummy_child_ancestry;

struct dummy_ancestry {
  using states_list_type = std::tuple<dummy_state>;
  constexpr static std::size_t generation_count() noexcept { return 0; }
  template <typename T>
  dummy_child_ancestry with_new_generation(T &&) const noexcept;
};

struct dummy_ancestry_with_accessors : dummy_ancestry {
  using origin_state_type = dummy_state;
  dummy_state const &origin() const noexcept;
  dummy_state &origin() noexcept;
  template <std::size_t I> dummy_state const &ancestor_at() const noexcept;
  template <std::size_t I> dummy_state &ancestor_at() noexcept;
  template <typename State> State const &ancestor_of() const noexcept;
  template <typename State> State &ancestor_of() noexcept;
};

struct dummy_parent_ancestry : dummy_ancestry_with_accessors {
  using parent_state_type = dummy_state;
  dummy_state const &parent() const noexcept;
  dummy_state &parent() noexcept;
};

struct dummy_child_ancestry : dummy_ancestry_with_accessors {
  using child_state_type = dummy_state;
  dummy_state const &child() const noexcept;
  dummy_state &child() noexcept;
};

struct dummy_full_ancestry : dummy_ancestry_with_accessors {
  using parent_state_type = dummy_state;
  using child_state_type = dummy_state;
  dummy_state const &child() const noexcept;
  dummy_state &child() noexcept;
  dummy_state const &parent() const noexcept;
  dummy_state &parent() noexcept;
};

struct dummy_machine_ancestry : dummy_ancestry {
  using machine_type =
      dummy_machine<std::tuple<dummy_state>, std::tuple<dummy_state>>;
  machine_type const &machine() const noexcept;
  machine_type &machine() noexcept;
};

template <typename T> struct dummy_child_ancestor {
  using origin_state_type = T;
  using child_state_type = T;
  using states_list_type = std::tuple<T>;
  std::size_t generation_count() const noexcept { return 1; }
  template <typename X>
  dummy_parent_ancestry with_new_generation(X &&) const noexcept {
    return {};
  }
  T const &child() const noexcept { return t; }
  T &child() noexcept { return t; }
  T const &origin() const noexcept { return t; }
  T &origin() noexcept { return t; }
  template <std::size_t I> T const &ancestor_at() const noexcept { return t; }
  template <std::size_t I> T &ancestor_at() noexcept { return t; }
  template <typename State> State const &ancestor_of() const noexcept {
    return t;
  }
  template <typename State> State &ancestor_of() noexcept { return t; }

  T t;
};

struct dummy_transition_coordinator {
  template <typename State>
  using self_transitions_for_state = std::index_sequence<>;

  template <std::size_t I, typename State, typename Machine>
  bool schedule_upon_acceptance(State const &, Machine const &);
};

struct dummy_state_container {
  using states_list_type = std::tuple<dummy_state>;
  template <typename State> bool is() const noexcept;
  template <typename Machine> void on_initial_entry(Machine &);
  template <typename Machine> void on_final_exit(Machine &);
  template <typename PreviousState, typename CurrentState, typename Machine,
            typename Event>
  void on_entry(Machine &, Event const &);
  template <typename State, typename Machine, typename Event>
  void on_reentry(Machine &, Event const &);
  template <typename CurrentState, typename NextState, typename Machine,
            typename Event>
  void on_exit(Machine &, Event const &);
  template <typename Machine, typename TransitionCoordinator>
  bool schedule_acceptable_transitions(Machine const &,
                                       TransitionCoordinator &) const;
};
} // namespace details_

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

template <typename T>
struct is_event
    : std::negation<std::disjunction<std::is_void<T>, std::is_pointer<T>>> {};
template <typename T>
struct is_event<T &> : is_event<std::remove_const_t<T>> {};

template <typename T>
struct is_state
    : std::conjunction<
          std::is_destructible<T>,
          std::negation<std::disjunction<std::is_void<T>, std::is_pointer<T>>>,
          std::disjunction<std::is_copy_constructible<T>,
                           std::is_move_constructible<T>>> {};
template <typename T>
struct is_state<T &> : is_state<std::remove_const_t<T>> {};

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

namespace is_transition_details_ {

template <typename, typename = void>
struct has_current_state_type : std::false_type {};
template <typename T>
struct has_current_state_type<T, std::void_t<typename T::current_state_type>>
    : is_state<typename T::current_state_type> {};

template <typename, typename = void>
struct has_next_state_type : std::false_type {};
template <typename T>
struct has_next_state_type<T, std::void_t<typename T::next_state_type>>
    : is_state<typename T::next_state_type> {};

template <typename, typename = void> struct has_event_type : std::false_type {};
template <typename T>
struct has_event_type<T, std::void_t<typename T::event_type>>
    : is_event<typename T::event_type> {};

template <typename, bool> struct has_accepts_impl : std::false_type {};
template <typename T>
requires requires(
    T const &ct, typename T::current_state_type const &cs,
    details_::dummy_machine<std::tuple<typename T::current_state_type>,
                            std::tuple<typename T::event_type>,
                            std::tuple<T>> const &cm,
    typename T::event_type const &ce) {
  { ct.accepts(cs, cm, ce) }
  noexcept->std::same_as<bool>;
}
struct has_accepts_impl<T, true> : std::true_type {};

template <typename T>
struct has_accepts
    : has_accepts_impl<
          T, std::conjunction_v<has_current_state_type<T>, has_event_type<T>>> {
};

template <typename, bool, typename = void>
struct has_on_triggered_impl : std::false_type {};
template <typename T>
struct has_on_triggered_impl<
    T, true,
    std::void_t<decltype(std::declval<T &>().on_triggered(
        std::declval<typename T::event_type const &>()))>> : std::true_type {};
template <typename T>
using has_on_triggered = has_on_triggered_impl<T, has_event_type<T>::value>;

template <typename, bool> struct is_self_transition : std::false_type {};

template <typename T>
struct is_self_transition<T, true>
    : std::is_same<typename T::current_state_type,
                   typename T::next_state_type> {};
} // namespace is_transition_details_

template <typename T>
using is_transition =
    std::conjunction<is_transition_details_::has_current_state_type<T>,
                     is_transition_details_::has_next_state_type<T>,
                     is_transition_details_::has_event_type<T>,
                     is_transition_details_::has_accepts<T>>;

template <typename T>
using is_actionable_transition =
    std::conjunction<is_transition<T>,
                     is_transition_details_::has_on_triggered<T>>;

template <typename T>
using is_self_transition =
    is_transition_details_::is_self_transition<T, is_transition<T>::value>;

template <typename T>
using is_transition_table =
    std::conjunction<is_template<T, std::tuple>, all<T, is_transition>>;

namespace is_machine_details_ {
template <bool, typename> struct has_signatures : std::false_type {};

template <typename T>
requires requires(T &t, T const &ct, details_::dummy_event const &cde) {
  {t.start()};
  {t.stop()};
  { t.on(cde) } -> std::same_as<bool>;
  { ct.template is<details_::dummy_state, details_::dummy_state>() }
  noexcept->std::same_as<bool>;
}
struct has_signatures<true, T> : std::true_type {};

template <typename, typename, bool>
struct contains_transition : std::false_type {};

template <typename Machine, typename Transition>
struct contains_transition<Machine, Transition, true>
    : contains<typename Machine::transition_table_type, Transition> {};
} // namespace is_machine_details_

template <typename T>
using is_machine = is_machine_details_::has_signatures<
    std::conjunction_v<std::is_destructible<T>,
                       std::disjunction<std::is_copy_constructible<T>,
                                        std::is_move_constructible<T>>,
                       has_states_list_type<T>>,
    T>;

template <typename Machine, typename Transition>
using is_machine_for =
    is_machine_details_::contains_transition<Machine, Transition,
                                             is_machine<Machine>::value>;

namespace is_ancestry_details_ {
template <typename T>
    struct has_generation_count : std::bool_constant < requires {
  { T::generation_count() }
  noexcept->std::same_as<std::size_t>;
} > {};

template <typename, typename = void, typename = void>
struct has_origin : std::false_type {};
template <typename T>
struct has_origin<
    T, std::enable_if_t<noexcept(std::declval<T &>().origin())>,
    std::enable_if_t<noexcept(std::declval<T const &>().origin())>>
    : std::conjunction<is_state<decltype(std::declval<T &>().origin())>,
                       is_state<decltype(std::declval<T &>().origin())>> {};

template <typename, typename = void, typename = void>
struct has_parent : std::false_type {};
template <typename T>
struct has_parent<
    T, std::enable_if_t<noexcept(std::declval<T &>().parent())>,
    std::enable_if_t<noexcept(std::declval<T const &>().parent())>>
    : std::conjunction<is_state<decltype(std::declval<T &>().parent())>,
                       is_state<decltype(std::declval<T &>().parent())>> {};

template <typename, typename = void, typename = void>
struct has_child : std::false_type {};
template <typename T>
struct has_child<T, std::enable_if_t<noexcept(std::declval<T &>().child())>,
                 std::enable_if_t<noexcept(std::declval<T const &>().child())>>
    : std::conjunction<is_state<decltype(std::declval<T &>().child())>,
                       is_state<decltype(std::declval<T &>().child())>> {};

template <typename, typename = void, typename = void>
struct has_machine : std::false_type {};
template <typename T>
struct has_machine<
    T, std::enable_if_t<noexcept(std::declval<T &>().machine())>,
    std::enable_if_t<noexcept(std::declval<T const &>().machine())>>
    : std::conjunction<
          is_machine<
              std::remove_cvref_t<decltype(std::declval<T &>().machine())>>,
          is_machine<
              std::remove_cvref_t<decltype(std::declval<T &>().machine())>>> {};

template <typename T>
using has_with_new_generation_result =
    std::conjunction<has_states_list_type<T>, has_origin<T>,
                     has_generation_count<T>>;

template <typename, typename = void>
struct has_with_new_generation : std::false_type {};
template <typename T>
struct has_with_new_generation<
    T, std::void_t<decltype(std::declval<T const &>().with_new_generation(
           std::declval<details_::dummy_state const &>()))>>
    : has_with_new_generation_result<
          decltype(std::declval<T const &>().with_new_generation(
              std::declval<details_::dummy_state const &>()))> {};
} // namespace is_ancestry_details_

template <typename T>
using is_ancestry =
    std::conjunction<has_states_list_type<T>,
                     is_ancestry_details_::has_generation_count<T>,
                     is_ancestry_details_::has_with_new_generation<T>>;

template <typename T>
using is_child_ancestry =
    std::conjunction<is_ancestry<T>, is_ancestry_details_::has_origin<T>,
                     is_ancestry_details_::has_child<T>>;

template <typename T>
using is_parent_ancestry =
    std::conjunction<is_ancestry<T>, is_ancestry_details_::has_origin<T>,
                     is_ancestry_details_::has_parent<T>>;

template <typename T>
using is_full_ancestry =
    std::conjunction<is_ancestry<T>, is_ancestry_details_::has_origin<T>,
                     is_ancestry_details_::has_child<T>,
                     is_ancestry_details_::has_parent<T>>;

template <typename T>
using is_machine_ancestry =
    std::conjunction<is_ancestry<T>, is_ancestry_details_::has_machine<T>>;

namespace is_transition_coordinator_details_ {
template <typename, typename = void>
struct has_template_self_transitions_for_state : std::false_type {};

template <typename T>
struct has_template_self_transitions_for_state<
    T, std::void_t<typename T::template self_transitions_for_state<
           details_::dummy_state>>>
    : is_integer_sequence<typename T::template self_transitions_for_state<
          details_::dummy_state>> {};

template <typename, typename = void>
struct has_schedule_upon_acceptance : std::false_type {};

template <typename T>
struct has_schedule_upon_acceptance<
    T, std::void_t<
           decltype(std::declval<T &>()
                        .template schedule_upon_acceptance<std::size_t{0}>(
                            std::declval<details_::dummy_state const &>(),
                            std::declval<
                                details_::dummy_machine_ancestry const &>()))>>
    : std::is_same<
          bool,
          decltype(std::declval<T &>()
                       .template schedule_upon_acceptance<std::size_t{0}>(
                           std::declval<details_::dummy_state const &>(),
                           std::declval<
                               details_::dummy_machine_ancestry const &>()))> {
};
} // namespace is_transition_coordinator_details_

template <typename T>
struct is_transition_coordinator
    : std::conjunction<
          is_transition_coordinator_details_::
              has_template_self_transitions_for_state<T>,
          is_transition_coordinator_details_::has_schedule_upon_acceptance<T>> {
};

namespace is_state_container_details_ {
template <typename T>
    struct has_is_template : std::bool_constant < requires(T const &ct) {
  { ct.template is<details_::dummy_state>() }
  noexcept->std::same_as<bool>;
} > {};

template <typename T>
    struct has_on_initial_entry
    : std::bool_constant <
      requires(T &t,
               details_::dummy_machine<typename T::states_list_type,
                                       std::tuple<details_::dummy_event>> &dm) {
  t.on_initial_entry(dm);
} > {};

template <typename T>
    struct has_on_final_exit
    : std::bool_constant <
      requires(T &t,
               details_::dummy_machine<typename T::states_list_type,
                                       std::tuple<details_::dummy_event>> &dm) {
  t.on_final_exit(dm);
} > {};

template <typename T>
concept is_child_ancestry_concept = is_child_ancestry<T>::value;

template <typename T>
    struct has_on_exit
    : std::bool_constant <
      requires(T &t,
               details_::dummy_machine<typename T::states_list_type,
                                       std::tuple<details_::dummy_event>> &dm,
               details_::dummy_event const &cde) {
  t.template on_exit<front_t<typename T::states_list_type>,
                     skizzay::fsm::details_::dummy_state>(dm, cde);
} > {};

template <typename T>
    struct has_on_entry
    : std::bool_constant <
      requires(T &t,
               details_::dummy_machine<typename T::states_list_type,
                                       std::tuple<details_::dummy_event>> &dm,
               details_::dummy_event const &cde) {
  t.template on_entry<details_::dummy_state,
                      front_t<typename T::states_list_type>>(dm, cde);
} > {};

template <typename T>
    struct has_on_reentry
    : std::bool_constant <
      requires(T &t,
               details_::dummy_machine<typename T::states_list_type,
                                       std::tuple<details_::dummy_event>> &dm,
               details_::dummy_event const &cde) {
  t.template on_reentry<front_t<typename T::states_list_type>>(dm, cde);
} > {};

template <typename T>
    struct has_schedule_acceptable_transitions
    : std::bool_constant <
      requires(
          T const &ct,
          details_::dummy_machine<typename T::states_list_type,
                                  std::tuple<details_::dummy_event>> const &cdm,
          details_::dummy_transition_coordinator &dtc) {
  { ct.schedule_acceptable_transitions(cdm, dtc) } -> std::same_as<bool>;
} > {};
} // namespace is_state_container_details_

template <typename T>
using is_state_container = std::conjunction<
    has_states_list_type<T>,
    is_state_container_details_::has_on_initial_entry<T>,
    is_state_container_details_::has_on_final_exit<T>,
    is_state_container_details_::has_on_exit<T>,
    is_state_container_details_::has_on_entry<T>,
    is_state_container_details_::has_on_reentry<T>,
    is_state_container_details_::has_schedule_acceptable_transitions<T>>;

namespace list_details_ {
template <bool, typename, typename...> struct list_impl;
template <typename Tag, typename... Ts> struct list_impl<true, Tag, Ts...> {
  using type = simple_type_list<Tag, Ts...>;
};

template <typename Tag, template <typename> typename Predicate, typename... Ts>
struct list : list_impl<all_v<std::tuple<Ts...>, Predicate>, Tag, Ts...> {};
} // namespace list_details_

template <typename... Ts>
using events_list =
    typename list_details_::list<struct events_list_tag, is_event, Ts...>::type;

template <typename... Ts>
using states_list =
    typename list_details_::list<struct states_list_tag, is_state, Ts...>::type;

template <typename... Ts>
using state_containers_list =
    typename list_details_::list<struct state_containers_list_tag,
                                 is_state_container, Ts...>::type;

namespace containers_containing_state_details_ {
template <typename StateContainersList, typename State> class impl {
  template <typename StateContainer>
  using test = contains<typename StateContainer::states_list_type, State>;

public:
  using type = filter_t<StateContainersList, test>;
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

template <typename T> using event_t = typename T::event_type;

template <typename T> using current_state_t = typename T::current_state_type;

template <typename T> using next_state_t = typename T::next_state_type;

namespace states_list_t_details_ {
template <typename, typename = void> struct states_list_t_impl {};
template <typename T>
struct states_list_t_impl<T, std::void_t<typename T::states_list_type>> {
  using type = typename T::states_list_type;
};
template <typename T>
struct states_list_t_impl<T, std::enable_if_t<is_transition_table<T>::value>> {
  using type = as_container_t<
      unique_t<concat_t<map_t<T, next_state_t>, map_t<T, current_state_t>>>,
      states_list>;
};
} // namespace states_list_t_details_

template <typename T>
using states_list_t =
    typename states_list_t_details_::states_list_t_impl<T>::type;

namespace events_list_t_details_ {
template <typename T, typename = void> struct events_list_t_impl {
  using type = typename T::events_list_type;
};
template <typename T>
struct events_list_t_impl<T, std::enable_if_t<is_transition_table<T>::value>> {
  using type = as_container_t<unique_t<map_t<T, event_t>>, events_list>;
};
} // namespace events_list_t_details_

template <typename T>
using events_list_t =
    typename events_list_t_details_::events_list_t_impl<T>::type;

template <typename T>
using transition_table_t = typename T::transition_table_type;

namespace transition_at_t_details_ {
template <std::size_t I, typename T, typename = void> struct impl {
  using type = std::remove_cvref_t<element_at_t<I, T>>;
};
template <std::size_t I, typename T>
struct impl<I, T, std::void_t<transition_table_t<T>>>
    : impl<I, transition_table_t<T>> {};
} // namespace transition_at_t_details_

template <std::size_t I, typename T>
using transition_at_t = typename transition_at_t_details_::impl<I, T>::type;

template <typename T> using origin_state_t = typename T::origin_state_type;

template <typename T> using parent_state_t = typename T::parent_state_type;

template <typename T> using child_state_t = typename T::child_state_type;

template <typename T> using machine_t = typename T::machine_type;

} // namespace skizzay::fsm
