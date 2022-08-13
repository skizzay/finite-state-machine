#pragma once

#include <stdexcept>

namespace skizzay::fsm {

struct state_transition_ambiguity : std::logic_error {
  using logic_error::logic_error;
};

} // namespace skizzay::fsm
