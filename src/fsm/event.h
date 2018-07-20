#pragma once

#include "fsm/traits.h"
#include "fsm/type_sequence.h"
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace details_ {

template<class> struct event_tags_holder {};

template<class> struct event_data_holder;

template<class ...EventData>
struct event_data_holder<utils::type_sequence<EventData...>> : EventData ... {
   constexpr event_data_holder(EventData &&... event_data) noexcept(
         std::conjunction_v<std::is_nothrow_constructible<EventData, decltype(std::forward<EventData>(event_data))>...>
       )
       : EventData{std::forward<EventData>(event_data)}... {}
};

template<class T> using is_event_data = std::negation<is_tag<T>>;
template<class ...Ts> using event_data_holder_t = event_data_holder<utils::filter_t<utils::type_sequence<Ts...>, is_event_data>>;
template<class ...Ts> using event_tags_holder_t = event_tags_holder<utils::filter_t<utils::type_sequence<Ts...>, is_tag>>;

}

template <class... TagsAndEventData>
struct event : details_::event_tags_holder_t<TagsAndEventData...>,
               details_::event_data_holder_t<TagsAndEventData...> {
   using details_::event_data_holder_t<TagsAndEventData...>::event_data_holder;
};

template<class ...EventDataOnly>
event(EventDataOnly ...) -> event<remove_cref_t<EventDataOnly>...>;

}
