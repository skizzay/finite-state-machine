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

   constexpr bool accepts(current_state_type const &current_state, event_type const &evt) const {
      return this->Guard::operator()(current_state, evt);
   }

   constexpr void on_triggered(event_type const &e) {
      this->TransactionAction::operator()(e);
   }
};

template<class FromState, class ToState, class Event>
using simple_transition = basic_transition<FromState, ToState, Event, details_::empty_action_t, details_::accept_all_events_t>;

template<class FromState, class ToState, class Event, class Action>
using action_transition = basic_transition<FromState, ToState, Event, Action, details_::accept_all_events_t>;

template<class FromState, class ToState, class Event, class Guard>
using guarded_transition = basic_transition<FromState, ToState, Event, details_::empty_action_t, Guard>;

template<class FromState, class ToState, class Event, class Action, class Guard>
using transition = basic_transition<FromState, ToState, Event, Action, Guard>;

}
