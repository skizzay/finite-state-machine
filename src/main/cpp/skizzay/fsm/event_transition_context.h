#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {
namespace is_event_transition_context_details_ {
template <typename, typename>
struct has_on_transition_template_member_function : std::false_type {};

template <typename T, concepts::transition Transition>
requires requires(T &t, Transition &transition, event_t<Transition> const &ec) {
  t.on_transition(transition, ec);
}
struct has_on_transition_template_member_function<T, Transition>
    : std::true_type {};

template <typename, typename>
struct has_get_transitions_template_member_function : std::false_type {};

template <typename T, concepts::state State>
requires requires(T &t, State const &sc) {
  { t.get_transitions(sc) }
  noexcept->concepts::transition_table;
}
struct has_get_transitions_template_member_function<T, State> : std::true_type {
};

template <typename, typename>
struct has_schedule_entry_template_member_function : std::false_type {};

template <typename T, concepts::state State>
requires requires(T &t) {
  { t.template schedule_entry<State>() }
  noexcept;
}
struct has_schedule_entry_template_member_function<T, State> : std::true_type {
};

} // namespace is_event_transition_context_details_

namespace concepts {
template <typename T>
concept event_transition_context = std::move_constructible<T> && requires {
  typename transition_table_t<T>;
  typename current_states_list_t<T>;
  typename next_states_list_t<T>;
} && all_v<current_states_list_t<T>,
           curry<is_event_transition_context_details_::
                     has_get_transitions_template_member_function,
                 T>::template type>
    &&all_v<next_states_list_t<T>,
            curry<is_event_transition_context_details_::
                      has_schedule_entry_template_member_function,
                  T>::template type>
        &&all_v<map_t<transition_table_t<T>, std::remove_cvref_t>,
                curry<is_event_transition_context_details_::
                          has_on_transition_template_member_function,
                      T>::template type>;
} // namespace concepts

template <typename T>
using is_event_transition_context =
    std::bool_constant<concepts::event_transition_context<T>>;

struct empty_event_transition_context {
  using transition_table_type = std::tuple<>;

  constexpr std::tuple<>
  get_transitions(concepts::state auto const &) const noexcept {
    return {};
  }

  template <concepts::state_in<next_states_list_t<transition_table_type>>>
  constexpr void schedule_entry() const noexcept {}

  constexpr void on_transition(concepts::transition auto const &,
                               concepts::event auto const &) const noexcept {}
};

} // namespace skizzay::fsm
