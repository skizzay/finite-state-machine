#pragma once

#include <memory>
#include <skizzay/fsm/concepts.h>
#include <skizzay/fsm/traits.h>
#include <skizzay/fsm/type_list.h>
#include <tuple>
#include <type_traits>

namespace skizzay::fsm {

namespace ancestor_details_ {

template <concepts::state...> struct with_state_types {};
template <concepts::state ChildState> struct with_state_types<ChildState> {
  using child_state_type = std::remove_cvref_t<ChildState>;
  using origin_state_type = child_state_type;
};
template <concepts::state ChildState, concepts::state ParentState,
          concepts::state... AncestorStates>
struct with_state_types<ChildState, ParentState, AncestorStates...> {
  using child_state_type = std::remove_cvref_t<ChildState>;
  using parent_state_type = std::remove_cvref_t<ParentState>;
  using origin_state_type = std::remove_cvref_t<element_at_t<
      sizeof...(AncestorStates), states_list<ParentState, AncestorStates...>>>;
};

template <concepts::state... States>
struct ancestors : with_state_types<States...> {
  using states_list_type = states_list<std::remove_cvref_t<States>...>;

private:
  template <typename T>
  using const_correct_t =
      std::conditional_t<any_v<states_list<States...>, std::is_const>,
                         std::add_const_t<T>, std::remove_const_t<T>>;
  using tuple_type = std::tuple<const_correct_t<States> *...>;

public:
  constexpr explicit ancestors(const_correct_t<States> &...states) noexcept
      : states_{std::addressof(states)...} {}

  constexpr explicit ancestors(tuple_type states) noexcept : states_{states} {}

  constexpr static std::size_t generation_count() noexcept {
    return sizeof...(States);
  }

  template <std::size_t I>
  requires(I < length_v<tuple_type>) constexpr std::remove_pointer_t<
      element_at_t<I, tuple_type>> &generation_at() const noexcept {
    return *std::get<I>(states_);
  }

  template <concepts::state State>
  requires(contains_v<tuple_type, State>) constexpr std::add_const_t<
      std::remove_reference_t<State>> &generation_of() const noexcept {
    return *std::get<const_correct_t<State> *>(states_);
  }

  constexpr decltype(auto) child() const noexcept
      requires(0 < length_v<tuple_type>) {
    return this->template generation_at<0>();
  }

  constexpr decltype(auto) parent() const noexcept
      requires(1 < length_v<tuple_type>) {
    return this->template generation_at<1>();
  }

  constexpr decltype(auto) origin() const noexcept
      requires(0 < length_v<tuple_type>) {
    return this->template generation_at<length_v<tuple_type> - 1>();
  }

  template <concepts::state State>
  constexpr ancestors<State, States...>
  with_new_generation(State &state) const noexcept {
    return {std::tuple_cat(std::tie(&state), states_)};
  }

  constexpr ancestors<std::add_const_t<States>...>
  with_immutability() const noexcept {
    return [this]<std::size_t... Is>(
        std::index_sequence<Is...> const) noexcept {
      return ancestors<std::add_const_t<States>...>{
          std::as_const(this->template generation_at<Is>())...};
    }
    (std::make_index_sequence<sizeof...(States)>{});
  }

private:
  tuple_type states_;
};
} // namespace ancestor_details_

template <concepts::state... States>
using ancestors = ancestor_details_::ancestors<States...>;

template <std::size_t I, typename... Ts>
constexpr decltype(auto) get(ancestors<Ts...> const &a) noexcept {
  return a.template ancestor_at<I>();
}

template <typename U, typename... Ts>
constexpr decltype(auto) get(ancestors<Ts...> const &a) noexcept {
  return a.template ancestor_of<U>();
}
} // namespace skizzay::fsm

namespace std {

template <typename... Ts>
struct tuple_size<skizzay::fsm::ancestors<Ts...>>
    : std::integral_constant<
          std::size_t, skizzay::fsm::ancestors<Ts...>::generation_count()> {};

template <std::size_t I, typename... Ts>
struct tuple_element<I, skizzay::fsm::ancestors<Ts...>> {
  using type = std::remove_reference_t<
      decltype(std::declval<skizzay::fsm::ancestors<Ts...>>()
                   .template ancestor_at<I>())>;
};

} // namespace std