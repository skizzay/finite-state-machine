#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

#include <utility>

namespace skizzay::fsm {

namespace enter_fn_details_ {
template <typename... Ts> void on_entry(Ts const &...) = delete;

struct enter_fn final {
  constexpr void operator()(concepts::state auto &,
                            concepts::event_context auto &) const noexcept {}
};
} // namespace enter_fn_details_

inline namespace enter_fn_ns {
inline constexpr enter_fn_details_::enter_fn enter = {};
}

} // namespace skizzay::fsm