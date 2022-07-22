#pragma once

#include "skizzay/fsm/accepts.h"
#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/event_handler.h"
#include "skizzay/fsm/events_list.h"
#include "skizzay/fsm/state_queryable.h"
#include "skizzay/fsm/states_list.h"
#include "skizzay/fsm/transition_table.h"

#include <concepts>
#include <type_traits>

namespace skizzay::fsm {

namespace concepts {
template <typename T>
concept machine = std::destructible<T> && std::copy_constructible<T> &&
    state_queryable<T> && event_handler<T> && requires(T &t, T const &tc) {
  typename transition_table_t<T>;
  {t.start()};
  {t.stop()};
  { tc.is_running() }
  noexcept->boolean;
  { tc.is_stopped() }
  noexcept->boolean;
} &&(!empty_v<transition_table_t<T>>);
} // namespace concepts

template <typename T>
using is_machine = std::bool_constant<concepts::machine<T>>;

namespace machine_state_details_ {
template <concepts::states_list StatesList, concepts::event Event>
struct is_accepted_impl {
  add_cref_t<Event> event_;
  bool has_been_accepted = false;

  constexpr bool is_done() const noexcept { return has_been_accepted; }

  constexpr void
  operator()(concepts::state_in<StatesList> auto const &state) const noexcept {
    has_been_accepted = has_been_accepted || accepts(state, event_);
  }
};
} // namespace machine_state_details_

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
constexpr bool is_accepted(Machine const &machine,
                           Event const &event) noexcept {
  machine_state_details_::is_accepted_impl<states_list_t<Machine>, Event>
      query_impl{event};
  machine.query(query_impl);
  return query_impl.has_been_accepted;
}

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
constexpr void on_entry(Machine &machine, Event const &event) noexcept {
  machine.start();
  machine.on(event);
}

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
constexpr void on_exit(Machine &machine, Event const &event) noexcept {
  machine.on(event);
  machine.stop();
}

template <concepts::machine Machine,
          concepts::event_in<events_list_t<Machine>> Event>
constexpr void on_reentry(Machine &machine, Event const &event) noexcept {
  machine.on(event);
}

} // namespace skizzay::fsm
