#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/entry_context.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/simple_transition.h"
#include "skizzay/fsm/state_accessible.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/type_list.h"

#include <tuple>
#include <type_traits>

namespace skizzay::fsm {
namespace is_state_container_details_ {
struct fake_event_transition_context {
  struct event_type {};
  struct state_type {};
  using states_list_type = states_list<state_type>;
  using events_list_type = events_list<event_type>;
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;

  event_type e;
  state_type s;
  [[no_unique_address]] simple_transition<state_type, state_type, event_type> t;

  constexpr event_type const &event() const noexcept { return e; }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return s;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return s;
  }

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &) noexcept {}

  constexpr std::tuple<simple_transition<state_type, state_type, event_type> &>
  get_transitions(state_type const &) noexcept {
    return {t};
  }

  template <concepts::state_in<next_states_list_t<transition_table_type>>>
  constexpr void schedule_entry() noexcept {}
};

struct fake_entry_context {
  struct event_type {};
  struct state_type {};

  using states_list_type = states_list<state_type>;
  using events_list_type = events_list<event_type>;
  using next_states_list_type = states_list_type;

  event_type e;
  state_type s;

  constexpr event_type const &event() const noexcept { return e; }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return s;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return s;
  }

  template <concepts::state_in<states_list_type>>
  constexpr bool is_scheduled() const noexcept {
    return false;
  }

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &) noexcept {}
};
} // namespace is_state_container_details_

namespace concepts {
template <typename T>
concept state_container = state_accessible<T> &&
    requires(T &t, T const &tc,
             is_state_container_details_::fake_event_transition_context &fetc,
             is_state_container_details_::fake_entry_context &fec) {
  { tc.is_active() }
  noexcept->concepts::boolean;
  { tc.is_inactive() }
  noexcept->concepts::boolean;
  t.on_entry(fec);
  { t.on_event(fetc) } -> concepts::boolean;
};
} // namespace concepts

template <typename T>
using is_state_container = std::bool_constant<concepts::state_container<T>>;

template <concepts::entry_context EntryContext,
          concepts::state_container StateContainer>
constexpr bool has_state_scheduled_for_entry(EntryContext const &entry_context,
                                             StateContainer const &) noexcept {
  constexpr auto const check_each_state =
      []<concepts::state... States>(
          EntryContext const &entry_context,
          states_list<States...> const) noexcept->bool {
    return (entry_context.template is_scheduled<States>() || ...);
  };
  return check_each_state(
      entry_context,
      intersection_of_t<
          as_container_t<next_states_list_t<EntryContext>, states_list>,
          as_container_t<states_list_t<StateContainer>, states_list>>{});
}

} // namespace skizzay::fsm
