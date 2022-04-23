#include "skizzay/fsm/transition.h"

#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct missing_event_type {
  using current_state_type = test_objects::test_state<0, 1>;
  using next_state_type = test_objects::test_state<0, 1>;
};

struct missing_current_state_type {
  using event_type = test_objects::test_event<0>;
  using next_state_type = test_objects::test_state<0, 1>;
};

struct missing_next_state_type {
  using event_type = test_objects::test_event<0>;
  using current_state_type = test_objects::test_state<0, 1>;
};

struct valid_transition {
  using event_type = test_objects::test_event<0>;
  using current_state_type = test_objects::test_state<0, 1>;
  using next_state_type = test_objects::test_state<1, 1>;
};

struct self_transition {
  using event_type = test_objects::test_event<0>;
  using current_state_type = test_objects::test_state<0, 1>;
  using next_state_type = current_state_type;
};
} // namespace

TEST_CASE("void is not a transition", "[unit][transition]") {
  REQUIRE_FALSE(concepts::transition<void>);
}

TEST_CASE("missing event type is not a transition", "[unit][transition]") {
  REQUIRE_FALSE(concepts::transition<missing_event_type>);
}

TEST_CASE("missing current state type is not a transition",
          "[unit][transition]") {
  REQUIRE_FALSE(concepts::transition<missing_current_state_type>);
}

TEST_CASE("missing next state type is not a transition", "[unit][transition]") {
  REQUIRE_FALSE(concepts::transition<missing_next_state_type>);
}

TEST_CASE("valid transition is a transition", "[unit][transition]") {
  REQUIRE(concepts::transition<valid_transition>);
}

TEST_CASE("non-transition is not a self-transition", "[unit][transition]") {
  REQUIRE_FALSE(concepts::self_transition<missing_next_state_type>);
}

TEST_CASE("valid self transition is a self-transition", "[unit][transition]") {
  REQUIRE(concepts::self_transition<self_transition>);
}