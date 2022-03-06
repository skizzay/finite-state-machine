#include "skizzay/fsm/event_engine.h"

#include "skizzay/fsm/events_list.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct missing_events_list_type {
  template <std::size_t I>
  constexpr void post_event(test_objects::test_event<I> const &) {}
};

struct missing_post_event {
  using events_list_type = events_list<test_objects::test_event<0>>;
};

struct valid_event_engine {
  using events_list_type = events_list<test_objects::test_event<0>>;

  constexpr void post_event(test_objects::test_event<0> const &) {}
};
} // namespace

TEST_CASE("missing events list type is not an event engine",
          "[unit][event_engine]") {
  REQUIRE_FALSE(is_event_engine<missing_events_list_type>::value);
  REQUIRE_FALSE(concepts::event_engine<missing_events_list_type>);
}

TEST_CASE(
    "missing events list type is an event engine for specified event type",
    "[unit][event_engine]") {
  REQUIRE(is_event_engine_for<missing_events_list_type,
                              test_objects::test_event<0>>::value);
  REQUIRE(concepts::event_engine_for<missing_events_list_type,
                                     test_objects::test_event<0>>);
}

TEST_CASE("missing post event is not an event engine", "[unit][event_engine]") {
  REQUIRE_FALSE(is_event_engine<missing_post_event>::value);
  REQUIRE_FALSE(concepts::event_engine<missing_post_event>);
}

TEST_CASE("valid event engine is an event engine", "[unit][event_engine]") {
  REQUIRE(is_event_engine<valid_event_engine>::value);
  REQUIRE(concepts::event_engine<valid_event_engine>);
}