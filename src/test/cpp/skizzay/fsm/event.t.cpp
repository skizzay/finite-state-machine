#include "skizzay/fsm/event.h"

#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

TEST_CASE("epsilon event is an event", "[unit][event]") {
  REQUIRE(is_event<epsilon_event_t>::value);
  REQUIRE(concepts::event<epsilon_event_t>);
}

TEST_CASE("initial entry event is an event", "[unit][event]") {
  REQUIRE(is_event<initial_entry_event_t>::value);
  REQUIRE(concepts::event<initial_entry_event_t>);
}

TEST_CASE("final exit event is an event", "[unit][event]") {
  REQUIRE(is_event<final_exit_event_t>::value);
  REQUIRE(concepts::event<final_exit_event_t>);
}

TEST_CASE("test event is an event", "[unit][event]") {
  REQUIRE(is_event<test_objects::test_event<0>>::value);
  REQUIRE(concepts::event<test_objects::test_event<0>>);
}

TEST_CASE("void* is not an event", "[unit][event]") {
  REQUIRE_FALSE(is_event<void *>::value);
  REQUIRE_FALSE(concepts::event<void *>);
}

TEST_CASE("void is not an event", "[unit][event]") {
  REQUIRE_FALSE(is_event<void>::value);
  REQUIRE_FALSE(concepts::event<void>);
}