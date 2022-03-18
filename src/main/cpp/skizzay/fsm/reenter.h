#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

namespace skizzay::fsm {

namespace reenter_fn_details_ {
template <typename... Ts> void on_reentry(Ts const &...) = delete;

struct reenter_fn final {
  constexpr void operator()(concepts::state auto &,
                            concepts::event_context auto &) const noexcept {}
};
} // namespace reenter_fn_details_

inline namespace reenter_fn_ns {
inline constexpr reenter_fn_details_::reenter_fn reenter = {};
}

} // namespace skizzay::fsm