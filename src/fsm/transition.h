#pragma once

#include "fsm/traits.h"
#include <utility>

namespace skizzay::fsm {

namespace details_ {
struct accept_all_events_t {
   template<class FromState, class Event>
   constexpr bool operator()(FromState const &, Event const &) const noexcept {
      return true;
   }
};

struct empty_action_t {
   template<class Event>
   constexpr void operator()(Event const &) const noexcept {
   }
};

}

template<class FromState, class ToState, class Event, class TransactionAction, class Guard>
class basic_transition : private TransactionAction, private Guard {
   static_assert(is_event_v<Event>);

public:
   using next_state_type = ToState;
   using current_state_type = FromState;
   using event_type = Event;

   constexpr basic_transition(TransactionAction action, Guard guard) :
      TransactionAction{std::move(action)},
      Guard{std::move(guard)}
   {
   }

   constexpr basic_transition(TransactionAction action) :
      TransactionAction{std::move(action)},
      Guard{}
   {
   }

   constexpr basic_transition(Guard guard) :
      TransactionAction{},
      Guard{std::move(guard)}
   {
   }

   constexpr basic_transition() :
      TransactionAction{},
      Guard{}
   {
   }

   constexpr bool accepts(current_state_type const &current_state, event_type const &evt) const
       noexcept(noexcept(std::declval<Guard const>()(current_state, evt))) {
      return this->Guard::operator()(current_state, evt);
   }

   constexpr void on_triggered(event_type const &e)
      noexcept(noexcept(std::declval<TransactionAction>()(e))) {
      this->TransactionAction::operator()(e);
   }
};

template<class FromState, class ToState, class Event>
using simple_transition = basic_transition<FromState, ToState, Event, details_::empty_action_t, details_::accept_all_events_t>;

template<class FromState, class ToState, class Event>
constexpr auto make_simple_transition() noexcept {
   return simple_transition<FromState, ToState, Event>{};
}

template<class FromState, class ToState, class Event, class Action>
using action_transition = basic_transition<FromState, ToState, Event, Action, details_::accept_all_events_t>;

template<class FromState, class ToState, class Event, class Action>
constexpr auto make_action_transition(Action &&action) {
   return action_transition<FromState, ToState, Event, remove_cref_t<Action>>{std::forward<Action>(action)};
}

template<class FromState, class ToState, class Event, class Guard>
using guarded_transition = basic_transition<FromState, ToState, Event, details_::empty_action_t, Guard>;

template <class FromState, class ToState, class Event, class Guard>
constexpr auto make_guarded_transition(Guard &&guard) {
   return guarded_transition<FromState, ToState, Event, remove_cref_t<Guard>> {std::forward<Guard>(guard)};
}

template <class FromState, class ToState, class Event, class Action, class Guard>
using transition = basic_transition<FromState, ToState, Event, Action, Guard>;

template <class FromState, class ToState, class Event, class Action, class Guard>
constexpr auto make_transition(Action &&action, Guard &&guard) {
   return transition<FromState, ToState, Event, remove_cref_t<Action>, remove_cref_t<Guard>>
   {std::forward<Action>(action), std::forward<Guard>(guard)};
}

}
