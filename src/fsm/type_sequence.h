#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace skizzay::utils {

template<std::size_t N> using size_constant = std::integral_constant<std::size_t, N>;

template<class F, class T> using apply_f = typename F::template apply<T>::type;

template<class ...> struct type_sequence {};

template<class, template<class> class> struct all_of;
template<class T, template<class> class P> constexpr bool all_of_v = all_of<T, P>::value;
template<class... Ts, template <class> class P> struct all_of<type_sequence<Ts...>, P> : std::conjunction<P<Ts>...> {};

template<class, template<class> class> struct any_of;
template<class T, template<class> class P> constexpr bool any_of_v = any_of<T, P>::value;
template<class... Ts, template <class> class P> struct any_of<type_sequence<Ts...>, P> : std::disjunction<P<Ts>...> {};

template<class T, template<class> class P> using none_of = std::negation<any_of<T, P>>;
template<class T, template<class> class P> constexpr bool none_of_v = none_of<T, P>::value;

template<class, template<class> class> struct count_of;
template<class T, template<class> class P> constexpr std::size_t count_of_v = count_of<T, P>::value;
template<template<class> class P> struct count_of<type_sequence<>, P> : size_constant<0> {};
template<class T, class ...Ts, template<class> class P> struct count_of<type_sequence<T, Ts...>, P> : size_constant<(P<T>::value ? 1 : 0) + count_of_v<type_sequence<Ts...>, P>> {};

template<class> struct length;
template<class T> constexpr std::size_t length_v = length<T>::value;
template<class ...Ts> struct length<type_sequence<Ts...>> : size_constant<sizeof...(Ts)> {};

template<class> struct empty;
template<class T> constexpr bool empty_v = empty<T>::value;
template<class ...Ts> struct empty<type_sequence<Ts...>> : std::bool_constant<(0 == sizeof...(Ts))> {};

template<class, class> struct contains;
template<class T, class U> constexpr bool contains_v = contains<T, U>::value;
template<class... Ts, class U> struct contains<type_sequence<Ts...>, U> : std::disjunction<std::is_same<Ts, U>...> {};

template<class, std::size_t> struct at;
template<class T, std::size_t I> using at_t = typename at<T, I>::type;
template<class T, class... Ts>
struct at<type_sequence<T, Ts...>, 0> { using type = T; };
template<class T, class ...Ts, std::size_t I> struct at<type_sequence<T, Ts...>, I> {
   static_assert(I < length_v<type_sequence<T, Ts...>>, "Index not in range");
   using type = at_t<type_sequence<Ts...>, I - 1>;
};

template<class, class, std::size_t=0> struct index_of;
template<class T, class U, std::size_t I=0> constexpr std::size_t index_of_v = index_of<T, U, I>::value;
template<class ...Ts, class U, std::size_t I>
struct index_of<type_sequence<Ts...>, U, I> : size_constant<I> {
   static_assert(!empty_v<type_sequence<Ts...>>, "Type not found");
};
template<class T, class ...Ts, class U, std::size_t I>
struct index_of<type_sequence<T, Ts...>, U, I> : std::conditional_t<std::is_same_v<T, U>, size_constant<I>, index_of<type_sequence<Ts...>, U, I+1>> {
};

template<class T> using back = at<T, length_v<T> - 1>;
template<class T> using back_t = typename back<T>::type;
template<class T> constexpr auto back_v = back<T>::value;

template<class T> using front = at<T, 0>;
template<class T> using front_t = typename front<T>::type;
template<class T> constexpr auto front_v = front<T>::value;

template<class ...> struct concat;
template<class ...Ts> using concat_t = typename concat<Ts...>::type;
template<class T> struct concat<T> { using type = T; };
template <class T, class U, class... Vs> struct concat<T, U, Vs...> { using type = concat_t<concat_t<T, U>, Vs...>; };
template<class ...Ts, class ...Us> struct concat<type_sequence<Ts...>, type_sequence<Us...>> { using type = type_sequence<Ts..., Us...>; };
template<class T, T ...Ts, class U, U ...Us>
struct concat<std::integer_sequence<T, Ts...>, std::integer_sequence<U, Us...>> {
   using type = std::integer_sequence<std::common_type_t<T, U>, Ts..., Us...>;
};

template<class, std::size_t> struct take;
template<class T, std::size_t N> using take_t = typename take<T, N>::type;
template<class T, class ...Ts, std::size_t N>
struct take<type_sequence<T, Ts...>, N> {
   using type = concat_t<type_sequence<T>, take_t<type_sequence<Ts...>, N - 1>>;
};
template<class ...Ts>
struct take<type_sequence<Ts...>, 0> {
   using type = type_sequence<>;
};

template<class, std::size_t> struct skip;
template<class T, std::size_t N> using skip_t = typename skip<T, N>::type;
template<class T, class ...Ts, std::size_t N>
struct skip<type_sequence<T, Ts...>, N> {
   using type = skip_t<type_sequence<Ts...>, N - 1>;
};
template<class ...Ts>
struct skip<type_sequence<Ts...>, 0> {
   using type = type_sequence<Ts...>;
};

template<class, class, template<class, class> class> struct right_fold;
template<class Ts, class Tn, template<class, class> class F> using right_fold_t = typename right_fold<Ts, Tn, F>::type;
template<class ...Ts, class Tn, template<class, class> class F>
struct right_fold<type_sequence<Ts...>, Tn, F> {
private:
   using ts_t = type_sequence<Ts...>;
public:
   using type = right_fold_t<take_t<ts_t, length_v<ts_t> - 2>, typename F<back_t<ts_t>, Tn>::type, F>;
};
template<class Tn, template<class, class> class F>
struct right_fold<type_sequence<>, Tn, F> {
   using type = Tn;
};

template<class, class, template<class, class> class> struct left_fold;
template<class T, class U, template<class, class> class F> using left_fold_t = typename left_fold<T, U, F>::type;
template<class T0, class Tn, class ...Ts, template<class, class> class F>
struct left_fold<T0, type_sequence<Tn, Ts...>, F> {
   using type = left_fold_t<typename F<T0, Tn>::type, type_sequence<Ts...>, F>;
};
template<class T0, template<class, class> class F> struct left_fold<T0, type_sequence<>, F> { using type = T0; };

template<class, template<class> class> struct map;
template<class T, template<class> class P> using map_t = typename map<T, P>::type;
template<class ...Ts, template<class> class P> struct map<type_sequence<Ts...>, P> { using type = type_sequence<P<Ts>...>; };

template<class T> struct as_type_sequence { using type = type_sequence<T>; };
template<class T> using as_type_sequence_t = typename as_type_sequence<T>::type;
template<class ...Ts> struct as_type_sequence<type_sequence<Ts...>> { using type = type_sequence<Ts...>; };
template<class I, I... Is> struct as_type_sequence<std::integer_sequence<I, Is...>> { using type = type_sequence<std::integral_constant<I, Is>...>; };

template<class, template<class> class> struct filter;
template<class T, template<class> class P> using filter_t = typename filter<T, P>::type;
template<class T, class... Ts, template<class> class P>
struct filter<type_sequence<T, Ts...>, P> {
   using type = concat_t<
       std::conditional_t<P<T>::value, type_sequence<T>, type_sequence<>>,
       filter_t<type_sequence<Ts...>, P>>;
};
template<template<class> class P> struct filter<type_sequence<>, P> { using type = type_sequence<>; };

template<class> struct reverse;
template<class T> using reverse_t = typename reverse<T>::type;
template<> struct reverse<type_sequence<>> { using type = type_sequence<>; };
template<class T, class ...Ts> struct reverse<type_sequence<T, Ts...>> { using type = concat_t<reverse_t<type_sequence<Ts...>>, type_sequence<T>>; };
template<class I> struct reverse<std::integer_sequence<I>> { using type = std::integer_sequence<I>; };
template<class I, I I0, I ...Is> struct reverse<std::integer_sequence<I, I0, Is...>> { using type = concat_t<reverse_t<std::integer_sequence<I, Is...>>, std::integer_sequence<I, I0>>; };

template<class> struct as_tuple;
template<class T> using as_tuple_t = typename as_tuple<T>::type;
template<class ...Ts> struct as_tuple<type_sequence<Ts...>> { using type = std::tuple<Ts...>; };
template<class I, I ...Is> struct as_tuple<std::integer_sequence<I, Is...>> { using type = std::tuple<std::integral_constant<I, Is>...>; };

template<class, class> struct zipped;
template<class T, class U> using zipped_t = typename zipped<T, U>::type;
template<class ...Ts, class ...Us>
struct zipped<type_sequence<Ts...>, type_sequence<Us...>> {
   static_assert(length_v<type_sequence<Ts...>> == length_v<type_sequence<Us...>>,
         "Sequences must be the same length");
   using type = type_sequence<type_sequence<Ts, Us>...>;
};

template<class> struct indexed;
template<class T> using indexed_t = typename indexed<T>::type;
template<class ...Ts>
struct indexed<type_sequence<Ts...>> {
private:
   using ts_t = type_sequence<Ts...>;
public:
   using type = zipped_t<as_type_sequence_t<std::make_index_sequence<length_v<ts_t>>>, ts_t>;
};

template<class, template<class> class> struct to_filtered_indices;
template<class T, template <class> class P> using to_filtered_indices_t = typename to_filtered_indices<T, P>::type;
template<class ...Ts, template <class> class P>
struct to_filtered_indices<utils::type_sequence<Ts...>, P> {
private:
   template<class, class> struct to_index;
   template<std::size_t ...Is, std::size_t I, class T>
   struct to_index<std::index_sequence<Is...>, type_sequence<utils::size_constant<I>, T>> {
      using type = std::index_sequence<Is..., I>;
   };
public:
   using type = left_fold_t< std::index_sequence<>, filter_t<indexed_t<type_sequence<Ts...>>, P>, to_index>;
};

}
