#pragma once

#include "skizzay/fsm/accepts.h"
#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/event_transition_context.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/query.h"
#include "skizzay/fsm/snapshot.h"
#include "skizzay/fsm/state_accessible.h"
#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/state_schedule.h"
#include "skizzay/fsm/state_transition_ambiguity.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/type_list.h"

#include <tuple>
#include <type_traits>

namespace skizzay::fsm {
namespace is_state_container_details_ {
struct event_type {};
struct state_type {};

struct fake_event_engine {
  using events_list_type = events_list<event_type>;

  constexpr void
  post_event(concepts::event_in<events_list_type> auto const &) noexcept {}
};

struct fake_event_transition_context {
  using transition_table_type =
      std::tuple<simple_transition<state_type, state_type, event_type>>;

  [[no_unique_address]] simple_transition<state_type, state_type, event_type> t;

  template <concepts::transition_in<transition_table_type> Transition>
  constexpr void on_transition(Transition &,
                               event_t<Transition> const &) noexcept {}

  constexpr std::tuple<simple_transition<state_type, state_type, event_type> &>
  get_transitions(state_type const &) noexcept {
    return {t};
  }

  template <concepts::state_in<next_states_list_t<transition_table_type>>>
  constexpr void schedule_entry() noexcept {}
};

struct fake_state_provider {
  using states_list_type = states_list<state_type>;

  event_type e;
  state_type s;

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return s;
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return s;
  }
};

struct fake_state_schedule {
  using next_states_list_type = states_list<state_type>;

  template <concepts::state_in<next_states_list_type>>
  constexpr bool is_scheduled() const noexcept {
    return false;
  }
};
} // namespace is_state_container_details_

namespace concepts {
template <typename T>
concept state_container = state_accessible<T> && snapshottable<T> &&
    requires(T &t, T const &tc,
             is_state_container_details_::fake_state_schedule const &fss,
             is_state_container_details_::event_type const &e,
             is_state_container_details_::fake_event_engine &fee,
             is_state_container_details_::fake_state_provider &fsp,
             is_state_container_details_::fake_event_transition_context &fetc) {
  { tc.is_active() }
  noexcept->concepts::boolean;
  { tc.is_inactive() }
  noexcept->concepts::boolean;
  t.on_entry(fss, e, fee, fsp);
  { t.on_event(fetc, e, fee, fsp) } -> concepts::boolean;
  { tc.query(no_op(true)) } -> concepts::boolean;
};

template <typename T, typename RootTransitionTable>
concept root_state_container = state_container<T> &&
    root_transition_table<RootTransitionTable> &&
    contains_all_v<states_list_t<T>, states_list_t<RootTransitionTable>> &&
    (length_v<states_list_t<T>> ==
     length_v<states_list_t<RootTransitionTable>>);
} // namespace concepts

template <typename F, concepts::state_container StateContainer>
requires requires {
  typename StateContainer::query_states_list_type;
} && concepts::states_list<typename StateContainer::query_states_list_type>
struct query_result<F, StateContainer>
    : query_result<F, typename StateContainer::query_states_list_type> {
};

template <typename T>
using is_state_container = std::bool_constant<concepts::state_container<T>>;

template <concepts::state_schedule StateSchedule,
          concepts::state_container StateContainer>
using candidate_states_list_t = intersection_of_t<
    as_container_t<next_states_list_t<StateSchedule>, states_list>,
    as_container_t<states_list_t<StateContainer>, states_list>>;

template <concepts::state_schedule StateSchedule,
          concepts::state_container StateContainer>
constexpr bool
has_state_scheduled_for_entry(StateSchedule const &state_schedule,
                              StateContainer const &) noexcept {
  return []<concepts::state... States>(
             StateSchedule const &state_schedule,
             states_list<States...> const) noexcept->bool {
    return (state_schedule.template is_scheduled<States>() || ...);
  }
  (state_schedule, candidate_states_list_t<StateSchedule, StateContainer>{});
}

namespace attempt_transitions_details_ {

enum class acceptance_type { unaccepted, reentered, exited };

constexpr acceptance_type
impl(concepts::event_transition_context auto &event_transition_context,
     concepts::state auto &state, concepts::event auto const &event,
     concepts::state_provider auto &state_provider) {
  acceptance_type acceptance = acceptance_type::unaccepted;
  auto attempt_to_transition =
      [&]<concepts::transition Transition>(Transition &transition) {
        if (accepts(std::as_const(transition), std::as_const(state), event,
                    std::as_const(state_provider))) {
          if constexpr (concepts::self_transition<Transition>) {
            switch (acceptance) {
            case acceptance_type::exited:
              throw state_transition_ambiguity{
                  "Self transition encountered but state has already exited"};

            default:
              acceptance = acceptance_type::reentered;
            }
          } else {
            switch (acceptance) {
            case acceptance_type::reentered:
              throw state_transition_ambiguity{
                  "State changing transition encountered but state has "
                  "already reentered"};

            default:
              acceptance = acceptance_type::exited;
            }
          }
          event_transition_context.on_transition(transition, event);
        }
      };
  std::apply(
      [attempt_to_transition](concepts::transition auto &...transitions) {
        (attempt_to_transition(transitions), ...);
      },
      event_transition_context.get_transitions(std::as_const(state)));
  return acceptance;
}

struct attempt_transitions_fn {
  template <concepts::event_transition_context EventTransitionContext,
            concepts::state State, std::invocable ExitCallback>
  constexpr bool operator()(EventTransitionContext &event_transition_context,
                            State &state, ExitCallback &&exit_callback) const {
    return (*this)(event_transition_context, state,
                   event_transition_context.event(), event_transition_context,
                   std::forward<ExitCallback>(exit_callback));
  }

  template <concepts::event_transition_context EventTransitionContext,
            concepts::state State, concepts::event Event,
            concepts::state_provider StateProvider, std::invocable ExitCallback>
  constexpr bool operator()(EventTransitionContext &event_transition_context,
                            State &state, Event const &event,
                            StateProvider &state_provider,
                            ExitCallback &&exit_callback) const {
    switch (impl(event_transition_context, state, event, state_provider)) {
    case acceptance_type::reentered:
      return true;

    case acceptance_type::exited:
      if constexpr (concepts::boolean<std::invoke_result_t<ExitCallback>>) {
        return std::forward<ExitCallback>(exit_callback)();
      } else {
        std::forward<ExitCallback>(exit_callback)();
        return true;
      }

    default:
      return false;
    }
  }
};
} // namespace attempt_transitions_details_

inline namespace attempt_transitions_fn_ {
inline constexpr attempt_transitions_details_::attempt_transitions_fn
    attempt_transitions = {};
}

constexpr void
execute_initial_entry(concepts::state_container auto &state_container,
                      concepts::event_engine auto &event_engine,
                      concepts::state_provider auto &state_provider) {
  state_container.on_entry(empty_state_schedule{}, initial_entry_event,
                           event_engine, state_provider);
}

constexpr bool
execute_final_exit(concepts::state_container auto &state_container,
                   concepts::event_engine auto &event_engine,
                   concepts::state_provider auto &state_provider) {
  empty_event_transition_context event_transition_context;
  return state_container.on_event(event_transition_context, final_exit_event,
                                  event_engine, state_provider);
}

template <concepts::root_transition_table TransitionTable,
          concepts::event_in<TransitionTable> Event>
constexpr concepts::transition_table auto
get_transition_table_for(TransitionTable &transition_table,
                         Event const &) noexcept {
  using transitions_list_type = as_container_t<
      filter_t<TransitionTable, curry<is_event_in, Event>::template type>,
      simple_type_list>;

  return []<concepts::transition... Transitions>(
      TransitionTable & transition_table,
      simple_type_list<Transitions...> const) noexcept {
    return std::tie(std::get<Transitions>(transition_table)...);
  }
  (transition_table, transitions_list_type{});
}

template <concepts::event_node_transition_table TransitionTable,
          concepts::state_in<current_states_list_t<TransitionTable>> State>
constexpr concepts::transition_table auto
get_transition_table_for(TransitionTable &transition_table,
                         State const &) noexcept {
  using transitions_list_type =
      as_container_t<filter_t<map_t<TransitionTable, std::remove_cvref_t>,
                              curry<is_current_state_in, State>::template type>,
                     simple_type_list>;

  return []<concepts::transition... Transitions>(
      TransitionTable & transition_table,
      simple_type_list<Transitions...> const) noexcept {
    return std::tie(std::get<Transitions &>(transition_table)...);
  }
  (transition_table, transitions_list_type{});
}

} // namespace skizzay::fsm
