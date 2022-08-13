#pragma once

#include "skizzay/fsm/boolean.h"

namespace skizzay::fsm {
namespace is_done_details_ {
bool is_done(auto const &...) = delete;

struct impl final {
  template <typename T>
  requires requires(T const &tc) {
    { tc.is_done() }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(T const &t) const noexcept { return t.is_done(); }
  template <typename T>
  requires requires(T const &tc) {
    { is_done(tc) }
    noexcept->concepts::boolean;
  }
  constexpr bool operator()(T const &t) const noexcept { return is_done(t); }
  constexpr bool operator()(auto const &) const noexcept { return false; }
};
} // namespace is_done_details_

inline namespace is_done_fn_ {
inline constexpr is_done_details_::impl const is_done = {};
}
} // namespace skizzay::fsm
