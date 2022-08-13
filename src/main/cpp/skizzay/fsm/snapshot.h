#pragma once

#include "skizzay/fsm/const_ref.h"

#include <concepts>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

template <typename> struct is_memento_nothrow : std::false_type {};

template <typename> struct is_recover_from_memento_nothrow : std::false_type {};

template <typename T>
constexpr bool is_memento_nothrow_v = is_memento_nothrow<T>::value;

template <typename T>
constexpr bool is_recover_from_memento_nothrow_v =
    is_recover_from_memento_nothrow<T>::value;

template <typename> struct memento_type {};

template <typename T> using memento_t = typename memento_type<T>::type;

namespace memento_details_ {

template <typename... Ts> void memento(Ts const &...) = delete;

struct memento_fn final {
  template <typename T>
  requires requires(T const &tc) {
    { tc.memento() } -> std::copyable;
  }
  constexpr auto operator()(T const &tc) const
      noexcept(noexcept(tc.memento())) {
    return tc.memento();
  }

  template <typename T>
  requires requires(T const &tc) {
    { memento(tc) } -> std::copyable;
  }
  constexpr auto operator()(T const &tc) const noexcept(noexcept(memento(tc))) {
    return memento(tc);
  }
};
} // namespace memento_details_

inline namespace memento_fn_ {
inline constexpr memento_details_::memento_fn memento = {};
}

template <typename T>
requires requires(T const &tc) {
  { skizzay::fsm::memento(tc) }
  noexcept->std::copyable;
}
struct is_memento_nothrow<T> : std::true_type {};

template <typename T>
requires requires(T const &tc) {
  { skizzay::fsm::memento(tc) } -> std::copyable;
}
struct memento_type<T> {
  using type = std::remove_cvref_t<decltype(skizzay::fsm::memento(
      std::declval<add_cref_t<T>>()))>;
};

namespace recover_from_memento_details_ {
template <typename... Ts, typename... Ms>
void recover_from_memento(Ts &..., Ms &&...) = delete;

struct recover_from_memento_fn final {
  template <typename T, std::convertible_to<memento_t<T>> Memento>
  requires requires(T &t, Memento &&m) {
    t.recover_from_memento((Memento &&) m);
  }
  constexpr void operator()(T &t, Memento &&memento) const noexcept(
      noexcept(t.recover_from_memento(std::forward<Memento>(memento)))) {
    t.recover_from_memento(std::forward<Memento>(memento));
  }

  template <typename T, std::convertible_to<memento_t<T>> Memento>
  requires requires(T &t, Memento &&m) {
    recover_from_memento(t, (Memento &&) m);
  }
  constexpr void operator()(T &t, Memento &&memento) const
      noexcept(noexcept(recover_from_memento(t,
                                             std::forward<Memento>(memento)))) {
    recover_from_memento(t, std::forward<Memento>(memento));
  }

  template <typename T, std::convertible_to<memento_t<T>> Memento>
  requires std::copyable<T> && std::constructible_from<T, Memento> &&(!(
      requires(T & t, Memento &&m) {
        recover_from_memento(t, (Memento &&) m);
      } ||
      requires(T & t, Memento &&m) {
        t.recover_from_memento((Memento &&) m);
      })) constexpr void
  operator()(T &t, Memento &&memento) const
      noexcept(std::is_nothrow_copy_assignable_v<T>
                   &&std::is_nothrow_constructible_v<T, Memento>) {
    t = T(std::forward<Memento>(memento));
  }
};
} // namespace recover_from_memento_details_

inline namespace recover_from_memento_fn_ {
inline constexpr recover_from_memento_details_::recover_from_memento_fn
    recover_from_memento = {};
}

namespace concepts {
template <typename T>
concept snapshottable = requires {
  typename memento_t<T>;
}
&&requires(T &t, memento_t<T> &&m) {
  skizzay::fsm::recover_from_memento(t, (memento_t<T> &&) m);
};
} // namespace concepts

template <concepts::snapshottable Snapshottable>
requires requires(Snapshottable &s, memento_t<Snapshottable> &&m) {
  { recover_from_memento(s, (memento_t<Snapshottable> &&) m) }
  noexcept;
}
struct is_recover_from_memento_nothrow<Snapshottable> : std::true_type {};

} // namespace skizzay::fsm
