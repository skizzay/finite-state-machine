#pragma once

#include "skizzay/fsm/event_context.h"
#include "skizzay/fsm/state.h"

namespace skizzay::fsm {

namespace exit_fn_details_ {
template <typename... Ts> void on_exit(Ts const &...) = delete;

struct exit_fn final {
  constexpr void operator()(concepts::state auto &,
                            concepts::event_context auto &) const noexcept {}
};
} // namespace exit_fn_details_

inline namespace exit_fn_ns {
inline constexpr exit_fn_details_::exit_fn exit = {};
}

} // namespace skizzay::fsm