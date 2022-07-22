#pragma once

#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/event_engine.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_provider.h"
#include "skizzay/fsm/state_schedule.h"
#include "skizzay/fsm/states_list.h"

namespace skizzay::fsm {

template <concepts::event Event, concepts::event_engine EventEngine,
          concepts::state_provider StateProvider,
          concepts::state_schedule StateSchedule>
struct entry_context_root {
  using events_list_type = events_list_t<EventEngine>;
  using states_list_type = states_list_t<StateProvider>;
  using next_states_list_type = next_states_list_t<StateSchedule>;

  constexpr explicit entry_context_root(Event const &event,
                                        EventEngine &event_engine,
                                        StateProvider &state_provider,
                                        StateSchedule &state_schedule) noexcept
      : event_{event}, event_engine_{event_engine},
        state_provider_{state_provider}, state_schedule_{state_schedule} {}

  constexpr add_cref_t<Event> event() const noexcept { return event_; }

  template <concepts::event_in<events_list_type> PostEvent>
  constexpr void post_event(PostEvent &&event) {
    event_engine_.post_event(std::forward<PostEvent>(event));
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State const &state() const noexcept {
    return state_provider_.template state<State>();
  }

  template <concepts::state_in<states_list_type> State>
  constexpr State &state() noexcept {
    return state_provider_.template state<State>();
  }

  template <concepts::state_in<next_states_list_type> State>
  constexpr bool is_scheduled() const noexcept {
    return state_schedule_.template is_scheduled<State>();
  }

private:
  add_cref_t<Event> event_;
  EventEngine &event_engine_;
  StateProvider &state_provider_;
  StateSchedule &state_schedule_;
};
} // namespace skizzay::fsm
