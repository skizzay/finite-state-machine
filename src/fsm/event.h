#pragma once

#include "fsm/traits.h"
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

namespace details_ {
template<class ...Ts>
struct event_impl {
   constexpr event_impl() noexcept {}
};

template<class T, class ...Ts>
struct event_impl<T, Ts...> : T, event_impl<Ts...> {
   template<class ...Args>
   constexpr event_impl(T &&t, Args &&...args) noexcept(
         std::is_nothrow_constructible_v<T, decltype(std::forward<T>(t))> &&
         std::is_nothrow_constructible_v<event_impl<Ts...>, decltype(std::forward<Args>(args))...>) :
      T{std::forward<T>(t)},
      event_impl<Ts...>{std::forward<Args>(args)...}
   {
   }
};

template<class T, class ...Ts>
struct event_impl<tag<T>, Ts...> : tag<T>, event_impl<Ts...> {
   template<class ...Args>
   constexpr event_impl(Args &&...args) noexcept(
         std::is_nothrow_constructible_v<event_impl<Ts...>, decltype(std::forward<Args>(args))...>) :
      tag<T>{},
      event_impl<Ts...>{std::forward<Args>(args)...}
   {
   }
};

template<class ...Ts> struct type_sequence {};

template<class, template<class> class> struct all_of;
template<class ...Ts, template<class> class P>
struct all_of<type_sequence<Ts...>, P> : std::conjunction<P<Ts>...> {};
template<class, template<class> class> struct any_of;
template<class ...Ts, template<class> class P>
struct any_of<type_sequence<Ts...>, P> : std::disjunction<P<Ts>...> {};
template<class T, template<class> class P> constexpr bool any_of_v = any_of<T, P>::value;
template<class T, template<class> class P> using none_of = std::negation<any_of<T, P>>;

}

template<class ...TagsAndEventData>
struct event : details_::event_impl<TagsAndEventData...> {
   template<class ...Args, std::enable_if_t<std::conjunction_v<
      details_::any_of<details_::type_sequence<TagsAndEventData...>, is_tag>,
      details_::none_of<details_::type_sequence<Args...>, is_tag>
      >, int> = 0>
   constexpr event(Args &&...args) noexcept(std::is_nothrow_constructible_v<
         details_::event_impl<TagsAndEventData...>,
         decltype(std::forward<Args>(args))...>) :
      details_::event_impl<TagsAndEventData...>{std::forward<Args>(args)...}
   {
   }

   constexpr event(TagsAndEventData &&...args) noexcept(std::is_nothrow_constructible_v<
         details_::event_impl<TagsAndEventData...>,
         decltype(std::forward<TagsAndEventData>(args))...>) :
      details_::event_impl<TagsAndEventData...>{std::forward<TagsAndEventData>(args)...}
   {
   }
};

template<class ...EventDataOnly>
event(EventDataOnly ...) -> event<remove_cref_t<EventDataOnly>...>;

}
