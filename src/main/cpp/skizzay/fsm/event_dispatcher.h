#pragma once

#include "skizzay/fsm/entry_context_root.h"
#include "skizzay/fsm/entry_state_scheduler.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_context_root.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/transition_table.h"

#include <functional>
#include <queue>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {
namespace event_dispatcher_details_ {
struct ignore_flag final {
  constexpr inline ignore_flag(bool const) noexcept {}
};

template <concepts::event Event> struct event_provider {
  static constexpr Event const &get() noexcept;
};

template <>
constexpr initial_entry_event_t const &
event_provider<initial_entry_event_t>::get() noexcept {
  return initial_entry_event;
}

template <>
constexpr final_exit_event_t const &
event_provider<final_exit_event_t>::get() noexcept {
  return final_exit_event;
}

template <concepts::state_provider StateProvider,
          concepts::event_engine EventEngine, concepts::event Event>
struct context_wrapper {
  using events_list_type = events_list_t<EventEngine>;
  using states_list_type = states_list_t<StateProvider>;
  using transition_table_type = std::tuple<>;

  constexpr explicit context_wrapper(StateProvider &state_provider,
                                     EventEngine &event_engine) noexcept
      : state_provider_{state_provider}, event_engine_{event_engine} {}

  constexpr add_cref_t<Event> event() const noexcept {
    return event_provider<std::remove_cvref_t<Event>>::get();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return state_provider_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return state_provider_.template state<State>();
  }

  template <concepts::event_in<events_list_type> PostEvent>
  constexpr void post_event(PostEvent &&event) {
    event_engine_.post_event(std::forward<PostEvent>(event));
  }

  constexpr std::tuple<>
  get_transitions(concepts::state auto const &) noexcept {
    return {};
  }

private:
  StateProvider &state_provider_;
  EventEngine &event_engine_;
};

template <concepts::event_engine Impl> struct event_engine_wrapper {
  using events_list_type = events_list_t<Impl>;

  constexpr explicit event_engine_wrapper(Impl &impl) noexcept : impl_{impl} {}

  constexpr void post_event(concepts::event_in<events_list_type> auto &&event) {
    impl_.post_event(std::forward<decltype(event)>(event));
  }

private:
  Impl &impl_;
};

template <concepts::state_provider Impl> struct state_provider_wrapper {
  using states_list_type = states_list_t<Impl>;

  constexpr explicit state_provider_wrapper(Impl &impl) noexcept
      : impl_{impl} {}

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return impl_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return impl_.template state<State>();
  }

private:
  Impl &impl_;
};
} // namespace event_dispatcher_details_

template <concepts::transition_table TransitionTable,
          concepts::state_container RootStateContainer>
struct event_dispatcher {
  using transition_table_type = TransitionTable;

  constexpr explicit event_dispatcher(transition_table_type &&transition_table,
                                      RootStateContainer &root_state_container)
      : q_{}, root_state_container_{root_state_container},
        transition_table_{
            std::forward<transition_table_type>(transition_table)},
        accept_epsilon_events_{true} {}

  template <concepts::event_in<transition_table_type> Event>
  constexpr void post_event(Event &&event) {
    if constexpr (std::is_same_v<std::remove_cvref_t<Event>, epsilon_event_t>) {
      accept_epsilon_events_ = false;
    }
    q_.emplace(
        [this, event = std::forward<Event>(event)]() { process_event(event); });
  }

  template <concepts::event Event>
  requires concepts::event_in<Event, transition_table_type> ||
      std::same_as<std::remove_cvref_t<Event>, initial_entry_event_t> ||
      std::same_as<std::remove_cvref_t<Event>, final_exit_event_t>
  constexpr bool dispatch_event(Event const &event) {
    if (process_event(event)) {
      process_internal_events();
      return true;
    } else {
      return false;
    }
  }

private:
  std::queue<std::function<void()>> q_;
  RootStateContainer &root_state_container_;
  [[no_unique_address]] transition_table_type transition_table_;
  [[no_unique_address]] std::conditional_t<
      concepts::event_in<epsilon_event_t, transition_table_type>, bool,
      event_dispatcher_details_::ignore_flag>
      accept_epsilon_events_;

  constexpr bool process_event(initial_entry_event_t const &) {
    event_dispatcher_details_::context_wrapper<
        RootStateContainer, std::remove_reference_t<decltype(*this)>,
        initial_entry_event_t>
        context_wrapper{root_state_container_, *this};
    execute_initial_entry(context_wrapper, root_state_container_);
    return true;
  }

  constexpr bool process_event(final_exit_event_t const &) {
    event_dispatcher_details_::context_wrapper<
        RootStateContainer, std::remove_reference_t<decltype(*this)>,
        final_exit_event_t>
        context_wrapper{root_state_container_, *this};
    return std::get<bool>(
        execute_final_exit(context_wrapper, root_state_container_));
  }

  template <concepts::event Event>
  requires concepts::event_in<Event, transition_table_type>
  constexpr bool process_event(Event const &event) {
    using next_states_list_type =
        as_container_t<map_t<filter_t<transition_table_type,
                                      curry<is_event_in, Event>::template type>,
                             next_state_t>,
                       states_list>;
    if constexpr (std::is_same_v<std::remove_cvref_t<Event>, epsilon_event_t>) {
      accept_epsilon_events_ = true;
    }
    entry_state_scheduler<next_states_list_type> scheduler;
    event_context_root event_context{event, *this, root_state_container_,
                                     transition_table_, scheduler};
    if (root_state_container_.on_event(event_context, event, *this,
                                       root_state_container_)) {
      root_state_container_.on_entry(scheduler, event, *this,

                                     root_state_container_);
      return true;
    } else {
      return false;
    }
  }

  void process_internal_events() {
    while (!std::empty(q_)) {
      std::function<void()> const callback{std::move(q_.front())};
      q_.pop();
      callback();
    }
  }
};

} // namespace skizzay::fsm
