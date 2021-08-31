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

template <typename, typename...> struct ancestors;

template <concepts::state... States>
struct ancestors<void, States...> : with_state_types<States...> {
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
  constexpr ancestors<void, State, States...>
  with_new_generation(State &state) const noexcept {
    return {std::tuple_cat(std::tie(&state), states_)};
  }

  constexpr ancestors<void, std::add_const_t<States>...>
  with_immutability() const noexcept {
    return [this]<std::size_t... Is>(
        std::index_sequence<Is...> const) noexcept {
      return ancestors<void, std::add_const_t<States>...>{
          std::as_const(this->template generation_at<Is>())...};
    }
    (std::make_index_sequence<sizeof...(States)>{});
  }

  template <concepts::machine Machine>
  constexpr ancestors<Machine, States...>
  with_machine(Machine &machine) const noexcept;

private:
  tuple_type states_;
};

template <concepts::machine Machine, concepts::state... States>
struct ancestors<Machine, States...>
    : public ancestors<
          void,
          std::conditional_t<std::is_const_v<Machine>, std::add_const_t<States>,
                             std::remove_const_t<States>>...> {
  using machine_type = std::remove_cvref_t<Machine>;

private:
  template <typename T>
  using const_correct_t =
      std::conditional_t<std::is_const_v<Machine>, std::add_const_t<T>,
                         std::remove_const_t<T>>;

public:
  constexpr ancestors(Machine &machine,
                      const_correct_t<States> &...states) noexcept
      : ancestors<void, const_correct_t<States>...>{states...},
        machine_{std::addressof(machine)} {}

  constexpr ancestors(
      Machine &machine,
      std::tuple<
          std::conditional_t<std::is_const_v<Machine>, std::add_const_t<States>,
                             std::remove_const_t<States>> &...>
          states) noexcept
      : ancestors<void, const_correct_t<States>...>{states},
        machine_{std::addressof(machine)} {}

  constexpr Machine &machine() const noexcept { return *machine_; }

  template <concepts::state State>
  constexpr ancestors<Machine, State, States...>
  with_new_generation(State &state) const noexcept {
    return [this]<std::size_t... Is>(std::index_sequence<Is...> const,
                                     const_correct_t<State> &state) noexcept {
      return ancestors<const_correct_t<Machine>, const_correct_t<State>,
                       const_correct_t<States>...>{
          machine(), state, this->template generation_at<Is>()...};
    }
    (std::make_index_sequence<sizeof...(States)>{}, state);
  }

  constexpr ancestors<std::add_const_t<Machine>, std::add_const_t<States>...>
  with_immutability() const noexcept {
    return [this]<std::size_t... Is>(
        std::index_sequence<Is...> const) noexcept {
      return ancestors<std::add_const_t<Machine>, std::add_const_t<States>...>{
          std::as_const(machine()),
          std::as_const(this->template generation_at<Is>())...};
    }
    (std::make_index_sequence<sizeof...(States)>{});
  }

  constexpr ancestors<void, States...> without_machine() const noexcept {
    return [this]<std::size_t... Is>(
        std::index_sequence<Is...> const) noexcept {
      return ancestors<void, States...>{this->template generation_at<Is>()...};
    }
    (std::make_index_sequence<sizeof...(States)>{});
  }

private:
  Machine *machine_;
};

template <concepts::state... States>
template <concepts::machine Machine>
constexpr ancestors<Machine, States...>
ancestors<void, States...>::with_machine(Machine &machine) const noexcept {
  return [this]<std::size_t... Is>(std::index_sequence<Is...> const,
                                   Machine &machine) noexcept {
    return ancestors<Machine, States...>{machine,
                                         this->template generation_at<Is>()...};
  }
  (std::make_index_sequence<sizeof...(States)>{}, machine);
}

template <typename... States> struct ancestors_type_picker {
  using type = ancestors<void, States...>;
};
template <concepts::machine Machine, concepts::state... States>
struct ancestors_type_picker<Machine, States...> {
  using type = ancestors<Machine, States...>;
};

template <typename... Ts>
using picked_ancestors_t = typename ancestors_type_picker<Ts...>::type;
} // namespace ancestor_details_

template <typename... Ts>
struct ancestors : ancestor_details_::picked_ancestors_t<Ts...> {
  using ancestor_details_::picked_ancestors_t<Ts...>::picked_ancestors_t;
};

template <std::size_t I, typename T, typename... Ts>
constexpr decltype(auto) get(ancestors<T, Ts...> const &a) noexcept {
  return a.template ancestor_at<I>();
}

template <typename U, typename T, typename... Ts>
constexpr decltype(auto) get(ancestors<T, Ts...> const &a) noexcept {
  return a.template ancestor_of<U>();
}

template <concepts::state... States>
ancestors(States &...) -> ancestors<States...>;

template <concepts::state... States>
ancestors(States const &...) -> ancestors<States const...>;

template <concepts::state... States>
ancestors(std::tuple<States &...> states) -> ancestors<States...>;

template <concepts::state... States>
ancestors(std::tuple<States const &...> states) -> ancestors<States const...>;

template <concepts::machine Machine, concepts::state... States>
ancestors(Machine &, States &...) -> ancestors<Machine, States...>;

template <concepts::machine Machine, concepts::state... States>
ancestors(Machine const &, States const &...)
    -> ancestors<Machine const, States const...>;

template <concepts::machine Machine, concepts::state... States>
ancestors(Machine &, std::tuple<States &...> states)
    -> ancestors<Machine, States...>;

template <concepts::machine Machine, concepts::state... States>
ancestors(Machine const &, std::tuple<States const &...> states)
    -> ancestors<Machine const, States const...>;

} // namespace skizzay::fsm

namespace std {

template <typename T, typename... Ts>
struct tuple_size<skizzay::fsm::ancestors<T, Ts...>>
    : std::integral_constant<
          std::size_t, skizzay::fsm::ancestors<T, Ts...>::generation_count()> {
};

template <std::size_t I, typename T, typename... Ts>
struct tuple_element<I, skizzay::fsm::ancestors<T, Ts...>> {
  using type = std::remove_reference_t<
      decltype(std::declval<skizzay::fsm::ancestors<T, Ts...>>()
                   .template ancestor_at<I>())>;
};

} // namespace std