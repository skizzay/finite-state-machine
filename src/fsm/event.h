#pragma once

#include "fsm/traits.h"
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

template<class Tag, class ...EventData>
struct event<tag<Tag>, EventData...> : EventData... {
   event(EventData && ...event_data) :
      EventData{std::forward<EventData>(event_data)}...
   {
   }
};

template<class ...EventData>
struct event : EventData... {
   event(EventData && ...event_data) :
      EventData{std::forward<EventData>(event_data)}...
   {
   }
};

template<class ...EventData>
event(EventData &&...) -> event<std::remove_const_t<std::remove_reference_t<EventData>>...>;

}
