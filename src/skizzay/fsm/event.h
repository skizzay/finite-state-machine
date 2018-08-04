#pragma once

#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>

namespace skizzay::fsm {

template<class ...TagsAndEventData>
struct event : utilz::dto<utilz::tag<struct event_tag>, TagsAndEventData...> {
   using utilz::dto<utilz::tag<struct event_tag>, TagsAndEventData...>::dto;
};

template<class ...Ts>
event(Ts &&...) -> event<utilz::remove_cref_t<Ts>...>;

}
