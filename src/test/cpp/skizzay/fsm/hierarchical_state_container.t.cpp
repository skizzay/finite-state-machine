#include <catch.hpp>
#include <skizzay/fsm/flat_state_container.h>
#include <skizzay/fsm/hierarchical_state_container.h>
#include <skizzay/fsm/single_state_container.h>

using namespace skizzay::fsm;

namespace {
template <std::size_t I> struct test_event {
  constexpr std::size_t index() const noexcept { return I; }
};
template <std::size_t I> struct test_state {
  constexpr std::size_t index() const noexcept { return I; }
};
} // namespace

SCENARIO("hierarchical state container with internal self transitions is a "
         "state container",
         "[unit][state-container][hierarchical-state-container]") {
  using target_type =
      parent_state_with_internal_self_transitions<
          test_state<0>,
          flat_state<single_state_container<test_state<1>>,
                               single_state_container<test_state<2>>>>;
  THEN("simple state container is a state container") {
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}

SCENARIO("hierarchical state container with external self transitions is a "
         "state container",
         "[unit][state-container][hierarchical-state-container]") {
  using target_type =
      parent_state_with_external_self_transitions<
          test_state<0>,
          flat_state<single_state_container<test_state<1>>,
                               single_state_container<test_state<2>>>>;
  THEN("simple state container is a state container") {
    REQUIRE(is_state_container<target_type>::value);
    REQUIRE(concepts::state_container<target_type>);
  }
}