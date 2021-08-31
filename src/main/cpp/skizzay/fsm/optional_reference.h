#pragma once

#include <memory>
#include <optional>
#include <skizzay/fsm/concepts.h>
#include <type_traits>
#include <utility>

namespace skizzay::fsm {

template <typename T> class optional_reference {
  T *t_ = nullptr;

public:
  constexpr optional_reference() noexcept = default;
  constexpr optional_reference(std::nullopt_t const) noexcept : t_{nullptr} {}
  constexpr optional_reference(T &t) noexcept : t_{std::addressof(t)} {}

  constexpr T *operator->() noexcept { return t_; }

  constexpr std::add_const_t<T> *operator->() const noexcept {
    return std::as_const(t_);
  }

  constexpr T &operator*() &noexcept { return *t_; }

  constexpr std::add_const_t<T> &operator*() const &noexcept { return *t_; }

  constexpr T &&operator*() &&noexcept { return std::move(*t_); }

  constexpr std::add_const_t<T> &&operator*() const &&noexcept {
    return std::move(std::as_const(*t_));
  }

  constexpr bool has_value() const noexcept { return nullptr != t_; }

  constexpr explicit operator bool() const noexcept { return has_value(); }

  constexpr T &value() & {
    if (has_value()) {
      return **this;
    } else {
      throw std::bad_optional_access{};
    }
  }

  constexpr std::add_const_t<T> &value() const & {
    if (has_value()) {
      return **this;
    } else {
      throw std::bad_optional_access{};
    }
  }

  constexpr T &&value() && {
    if (has_value()) {
      return std::move(**this);
    } else {
      throw std::bad_optional_access{};
    }
  }

  constexpr std::add_const_t<T> &&value() const && {
    if (has_value()) {
      return std::move(**this);
    } else {
      throw std::bad_optional_access{};
    }
  }

  constexpr operator T &() & { return value(); }
  constexpr operator T const &() const & { return value(); }
  constexpr operator T &&() && { return value(); }
  constexpr operator T const &&() const && { return value(); }
};

} // namespace skizzay::fsm
