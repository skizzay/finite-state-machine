#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>
#include <variant>

namespace skizzay::fsm {

template <typename...> struct simple_type_list {};

template <typename, template <typename> typename> struct all;
template <typename T, template <typename> typename Template>
inline constexpr bool const all_v = all<T, Template>::value;
template <template <typename...> typename Template, typename... Elements,
          template <typename> typename Predicate>
struct all<Template<Elements...>, Predicate>
    : std::conjunction<Predicate<Elements>...> {};

template <typename, template <typename> typename> struct any;
template <typename T, template <typename> typename Template>
inline constexpr bool const any_v = any<T, Template>::value;
template <template <typename...> typename Template, typename... Elements,
          template <typename> typename Predicate>
struct any<Template<Elements...>, Predicate>
    : std::disjunction<Predicate<Elements>...> {};

template <typename T, template <typename> typename Predicate>
using none = std::negation<any<T, Predicate>>;
template <typename T, template <typename> typename Predicate>
inline constexpr bool const none_v = none<T, Predicate>::value;

template <typename, typename> struct contains;
template <typename T, typename U>
inline constexpr bool const contains_v = contains<T, U>::value;
template <typename Element, template <typename...> typename Template,
          typename... Elements>
struct contains<Template<Elements...>, Element>
    : std::disjunction<std::is_same<Element, Elements>...> {};
template <std::integral Integral, Integral I, Integral... Is>
struct contains<std::integer_sequence<Integral, Is...>,
                std::integral_constant<Integral, I>>
    : std::bool_constant<((I == Is) || ...)> {};

namespace index_of_details_ {
template <typename, typename, std::size_t> struct impl;
template <template <typename...> typename Template, typename T, typename... Ts,
          typename U, std::size_t I>
struct impl<Template<T, Ts...>, U, I> {
  using type = typename impl<Template<Ts...>, U, I + 1>::type;
};
template <template <typename...> typename Template, typename T, typename... Ts,
          std::size_t I>
struct impl<Template<T, Ts...>, T, I> {
  using type = std::integral_constant<std::size_t, I>;
};
} // namespace index_of_details_

template <typename T, typename U>
using index_of = typename index_of_details_::impl<T, U, 0>::type;
template <typename T, typename U>
inline constexpr std::size_t const index_of_v = index_of<T, U>::value;

template <typename, typename> struct as_index_sequence;
template <typename T, typename U>
using as_index_sequence_t = typename as_index_sequence<T, U>::type;
template <template <typename...> typename Template, typename... Ts,
          typename SuperSet>
struct as_index_sequence<Template<Ts...>, SuperSet> {
  using type = std::index_sequence<index_of_v<Ts, SuperSet>...>;
};

template <typename, typename> struct contains_all;
template <typename T, typename U>
inline constexpr bool const contains_all_v = contains_all<T, U>::value;
template <typename ContainerList,
          template <typename...> typename CandidateTemplate,
          typename... Candidates>
struct contains_all<ContainerList, CandidateTemplate<Candidates...>>
    : std::conjunction<contains<ContainerList, Candidates>...> {};

template <typename, typename> struct contains_any;
template <typename T, typename U>
inline constexpr bool const contains_any_v = contains_any<T, U>::value;
template <typename ContainerList,
          template <typename...> typename CandidateTemplate,
          typename... Candidates>
struct contains_any<ContainerList, CandidateTemplate<Candidates...>>
    : std::disjunction<contains<ContainerList, Candidates>...> {};

template <typename, template <typename...> typename> struct as_container;
template <typename T, template <typename...> typename Template>
using as_container_t = typename as_container<T, Template>::type;
template <template <typename...> typename TargetContainer,
          template <typename...> typename SourceContainer, typename... Elements>
struct as_container<SourceContainer<Elements...>, TargetContainer> {
  using type = TargetContainer<Elements...>;
};

template <typename> struct length;
template <typename T>
inline constexpr std::size_t const length_v = length<T>::value;
template <std::integral Integral, Integral... Is>
struct length<std::integer_sequence<Integral, Is...>>
    : std::integral_constant<std::size_t, sizeof...(Is)> {};
template <template <typename...> typename Template, typename... Elements>
struct length<Template<Elements...>>
    : std::integral_constant<std::size_t, sizeof...(Elements)> {};

template <typename T>
struct empty : std::bool_constant<0 == length<T>::value> {};
template <typename T> inline constexpr bool const empty_v = empty<T>::value;

template <std::size_t I, typename List> struct element_at {
  using type = std::tuple_element_t<I, as_container_t<List, std::tuple>>;
};
template <std::size_t I, typename... Ts>
struct element_at<I, std::tuple<Ts...>>
    : std::tuple_element<I, std::tuple<Ts...>> {};
template <std::size_t I, typename... Ts>
struct element_at<I, std::variant<Ts...>>
    : std::variant_alternative<I, std::variant<Ts...>> {};
template <std::size_t I, typename List>
using element_at_t = typename element_at<I, List>::type;
template <typename List> using front = element_at<0, List>;
template <typename List> using front_t = typename front<List>::type;
template <typename List> using back = element_at<(length_v<List> - 1), List>;
template <typename List> using back_t = typename back<List>::type;

template <typename...> struct concat;
template <typename... Ts> using concat_t = typename concat<Ts...>::type;
template <typename T> struct concat<T> { using type = T; };
template <template <typename...> typename Template, typename... Elements1,
          typename... Elements2, typename... OtherLists>
struct concat<Template<Elements1...>, Template<Elements2...>, OtherLists...> {
  using type = concat_t<Template<Elements1..., Elements2...>, OtherLists...>;
};
template <std::integral Integral, Integral... Is, Integral... Js,
          typename... OtherSequences>
struct concat<std::integer_sequence<Integral, Is...>,
              std::integer_sequence<Integral, Js...>, OtherSequences...> {
  using type = concat_t<std::integer_sequence<Integral, Is..., Js...>,
                        OtherSequences...>;
};

template <std::size_t N, typename IntegralList> struct value_at;
template <std::size_t N, typename IntegralList>
inline constexpr typename IntegralList::value_type value_at_v =
    value_at<N, IntegralList>::value;
template <std::integral Integral, Integral I, Integral... Is,
          template <Integral...> typename IntegralTemplate>
struct value_at<0, IntegralTemplate<I, Is...>>
    : std::integral_constant<Integral, I> {};
template <std::size_t N, std::integral Integral, Integral I, Integral... Is,
          template <Integral...> typename IntegralTemplate>
struct value_at<N, IntegralTemplate<I, Is...>>
    : value_at<N - 1, IntegralTemplate<Is...>> {};

template <std::size_t, typename> struct skip;
template <std::size_t N, typename T> using skip_t = typename skip<N, T>::type;
template <std::size_t N, template <typename...> typename Container, typename T,
          typename... Ts>
struct skip<N, Container<T, Ts...>> {
  using type = skip_t<N - 1, Container<Ts...>>;
};
template <template <typename...> typename Container, typename T, typename... Ts>
struct skip<0, Container<T, Ts...>> {
  using type = Container<T, Ts...>;
};
template <template <typename...> typename Container, typename... Ts>
struct skip<0, Container<Ts...>> {
  using type = Container<Ts...>;
};

template <std::size_t N, std::integral Integral, Integral I, Integral... Is>
struct skip<N, std::integer_sequence<Integral, I, Is...>> {
  using type = skip_t<N - 1, std::integer_sequence<Integral, Is...>>;
};
template <std::integral Integral, Integral I, Integral... Is>
struct skip<0, std::integer_sequence<Integral, I, Is...>> {
  using type = std::integer_sequence<Integral, I, Is...>;
};
template <std::integral Integral, Integral... Is>
struct skip<0, std::integer_sequence<Integral, Is...>> {
  using type = std::integer_sequence<Integral, Is...>;
};

template <std::size_t, typename> struct take;
template <std::size_t I, typename T> using take_t = typename take<I, T>::type;
template <std::size_t N, template <typename...> typename Template, typename T,
          typename... Ts>
struct take<N, Template<T, Ts...>> {
  using type = concat_t<Template<T>, take_t<N - 1, Template<Ts...>>>;
};
template <template <typename...> typename Template, typename T, typename... Ts>
struct take<0, Template<T, Ts...>> {
  using type = Template<>;
};
template <template <typename...> typename Template, typename... Ts>
struct take<0, Template<Ts...>> {
  using type = Template<>;
};

template <std::size_t N, std::integral Integral, Integral I, Integral... Is>
struct take<N, std::integer_sequence<Integral, I, Is...>> {
  using type = concat_t<std::integer_sequence<Integral, I>,
                        take_t<N - 1, std::integer_sequence<Integral, Is...>>>;
};
template <std::integral Integral, Integral I, Integral... Is>
struct take<0, std::integer_sequence<Integral, I, Is...>> {
  using type = std::integer_sequence<Integral>;
};
template <std::integral Integral, Integral... Is>
struct take<0, std::integer_sequence<Integral, Is...>> {
  using type = std::integer_sequence<Integral>;
};

template <typename T> using pop_front = skip<1, T>;
template <typename T> using pop_front_t = typename pop_front<T>::type;
template <typename T> using pop_back = take<length<T>::value - 1, T>;
template <typename T> using pop_back_t = typename pop_back<T>::type;

template <typename, template <typename> typename> struct map;
template <typename T, template <typename> typename F>
using map_t = typename map<T, F>::type;
template <template <typename> typename F,
          template <typename...> typename Template, typename... Elements>
requires requires { typename std::void_t<typename F<Elements>::type...>; }
struct map<Template<Elements...>, F> {
  using type = Template<typename F<Elements>::type...>;
};
template <template <typename> typename F,
          template <typename...> typename Template, typename... Elements>
struct map<Template<Elements...>, F> {
  using type = Template<F<Elements>...>;
};

template <typename> struct flat_map;
template <typename T> using flat_map_t = typename flat_map<T>::type;
template <template <typename...> typename Template, typename... Ts>
struct flat_map<Template<Ts...>> {
  using type = concat_t<Ts...>;
};

template <typename, typename, template <typename, typename> typename>
struct left_fold;
template <typename T0, typename Ts, template <typename, typename> typename F>
using left_fold_t = typename left_fold<T0, Ts, F>::type;
template <typename T0, template <typename...> typename Template, typename Tn,
          typename... Ts, template <typename, typename> typename F>
struct left_fold<T0, Template<Tn, Ts...>, F> {
  using type = left_fold_t<typename F<T0, Tn>::type, Template<Ts...>, F>;
};
template <typename T0, template <typename...> typename Template,
          template <typename, typename> typename F>
struct left_fold<T0, Template<>, F> {
  using type = T0;
};

template <typename, template <typename> typename> struct filter;
template <typename T, template <typename> typename Predicate>
using filter_t = typename filter<T, Predicate>::type;
template <template <typename...> typename Template, typename Element,
          typename... Elements, template <typename> typename Predicate>
struct filter<Template<Element, Elements...>, Predicate> {
  using type = concat_t<std::conditional_t<Predicate<Element>::value,
                                           Template<Element>, Template<>>,
                        filter_t<Template<Elements...>, Predicate>>;
};
template <template <typename...> typename Template,
          template <typename> typename Predicate>
struct filter<Template<>, Predicate> {
  using type = Template<>;
};

template <template <typename> typename Predicate> struct negate {
  template <typename T> using apply = std::negation<Predicate<T>>;
};

template <typename, typename> struct remove;
template <typename T, typename U> using remove_t = typename remove<T, U>::type;

namespace details_ {
template <typename, typename> struct unique_impl;
template <template <typename...> typename Template, typename... Found>
struct unique_impl<Template<>, Template<Found...>> {
  using type = Template<Found...>;
};
template <template <typename...> typename Template, typename Element,
          typename... Elements, typename... Found>
struct unique_impl<Template<Element, Elements...>, Template<Found...>> {
  using type = typename unique_impl<
      Template<Elements...>,
      std::conditional_t<contains_v<Template<Found...>, Element>,
                         Template<Found...>,
                         Template<Found..., Element>>>::type;
};
} // namespace details_

template <typename> struct unique;
template <typename T> using unique_t = typename unique<T>::type;
template <template <typename...> typename Template, typename... Elements>
struct unique<Template<Elements...>>
    : details_::unique_impl<Template<Elements...>, Template<>> {};

namespace intersection_of_details_ {
template <typename T> struct contained {
  template <typename U> using in = contains<T, U>;
};
template <std::integral Integral, Integral... Is>
struct contained<std::integer_sequence<Integral, Is...>> {
  template <std::size_t I>
  using in = contains<std::integer_sequence<Integral, Is...>,
                      std::integral_constant<Integral, I>>;
};
} // namespace intersection_of_details_

template <typename...> struct intersection_of;
template <typename... Ts>
using intersection_of_t = typename intersection_of<Ts...>::type;
template <typename T> struct intersection_of<T> { using type = T; };
template <template <typename...> typename Template, typename... Ts,
          typename... Us>
struct intersection_of<Template<Ts...>, Template<Us...>> {
  using type = filter_t<Template<Ts...>, intersection_of_details_::contained<
                                             Template<Us...>>::template in>;
};
template <std::integral Integral, Integral... Ts, Integral... Us>
struct intersection_of<std::integer_sequence<Integral, Ts...>,
                       std::integer_sequence<Integral, Us...>> {
  using type =
      filter_t<std::integer_sequence<Integral, Ts...>,
               intersection_of_details_::contained<
                   std::integer_sequence<Integral, Us...>>::template in>;
};
template <typename T0, typename T1, typename... Ts>
struct intersection_of<T0, T1, Ts...> {
  using type = typename intersection_of<typename intersection_of<T0, T1>::type,
                                        Ts...>::type;
};

} // namespace skizzay::fsm