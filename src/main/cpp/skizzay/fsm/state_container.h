#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/entry_context.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/simple_transition.h"
#include "skizzay/fsm/state_accessible.h"
#include "skizzay/fsm/state_provider.h"
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
using candidate_states_list_t = intersection_of_t<
    as_container_t<next_states_list_t<EntryContext>, states_list>,
    as_container_t<states_list_t<StateContainer>, states_list>>;

template <concepts::entry_context EntryContext,
          concepts::state_container StateContainer>
constexpr bool has_state_scheduled_for_entry(EntryContext const &entry_context,
                                             StateContainer const &) noexcept {
  return []<concepts::state... States>(
             EntryContext const &entry_context,
             states_list<States...> const) noexcept->bool {
    return (entry_context.template is_scheduled<States>() || ...);
  }
  (entry_context, candidate_states_list_t<EntryContext, StateContainer>{});
}

namespace execute_intial_entry_details_ {
template <concepts::event_engine T>
requires concepts::state_provider<T>
struct initial_entry_context {
  using states_list_type = states_list_t<T>;
  using events_list_type = events_list_t<T>;
  using event_type = initial_entry_event_t;
  using current_states_list_type = states_list<>;
  using next_states_list_type = states_list_type;

  constexpr explicit initial_entry_context(T &t) noexcept : t_{t} {}

  constexpr initial_entry_event_t const &event() const noexcept {
    return initial_entry_event;
  }

  template <concepts::state_in<next_states_list_type>>
  constexpr bool is_scheduled() const noexcept {
    return false;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return t_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return t_.template state<State>();
  }

  template <concepts::event_in<events_list_type> Event>
  constexpr void post_event(Event const &event) {
    t_.post_event(event);
  }

private:
  T &t_;
};
} // namespace execute_intial_entry_details_

template <concepts::event_engine T>
requires concepts::state_provider<T>
constexpr auto
execute_initial_entry(T &entry_context,
                      concepts::state_container auto &state_container) {
  execute_intial_entry_details_::initial_entry_context initial_entry_context{
      entry_context};
  state_container.on_entry(initial_entry_context);
  return initial_entry_context;
}

namespace execute_final_exit_details_ {
template <concepts::event_engine T>
requires concepts::state_provider<T>
struct final_event_transition_context {
  using event_type = final_exit_event_t;
  using events_list_type = events_list_t<T>;
  using states_list_type = states_list_t<T>;
  using transition_table_type = std::tuple<>;

  constexpr explicit final_event_transition_context(T &t) noexcept : t_{t} {}

  constexpr event_type const &event() const noexcept {
    return final_exit_event;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return t_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return t_.template state<State>();
  }

  template <concepts::event_in<events_list_type> Event>
  constexpr void post_event(Event const &event) {
    t_.post_event(event);
  }

  constexpr std::tuple<>
  get_transitions(concepts::state auto const &) noexcept {
    return {};
  }

private:
  T &t_;
};
} // namespace execute_final_exit_details_

template <concepts::event_engine T>
requires concepts::state_provider<T>
constexpr auto
execute_final_exit(T &entry_context,
                   concepts::state_container auto &state_container) {
  execute_final_exit_details_::final_event_transition_context
      final_event_transition_context{entry_context};
  bool const result = state_container.on_event(final_event_transition_context);
  return std::tuple{final_event_transition_context, result};
}

} // namespace skizzay::fsm
