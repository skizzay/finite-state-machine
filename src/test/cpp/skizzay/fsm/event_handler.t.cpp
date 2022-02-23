#include "skizzay/fsm/event_handler.h"

#include "skizzay/fsm/events_list.h"
#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct missing_events_list_type {
  template <typename T> constexpr bool on(T const &) noexcept { return false; }
};

struct missing_handler_for_single_event {
  using events_list_type =
      events_list<test_objects::test_event<0>, test_objects::test_event<1>>;

  constexpr bool on(test_objects::test_event<0> const &) noexcept {
    return false;
  }
};

struct valid_event_handler {
  using events_list_type =
      events_list<test_objects::test_event<0>, test_objects::test_event<1>>;

  template <concepts::event Event>
  requires contains_v<events_list_type, Event>
  constexpr bool on(Event const &) noexcept { return false; }
};
} // namespace

TEST_CASE("missing events list type is not an event handler",
          "[unit][event_handler]") {
  REQUIRE_FALSE(is_event_handler<missing_events_list_type>::value);
  REQUIRE_FALSE(concepts::event_handler<missing_events_list_type>);
}

TEST_CASE("not handling all event types is not an event handler",
          "[unit][event_handler]") {
  REQUIRE_FALSE(is_event_handler<missing_handler_for_single_event>::value);
  REQUIRE_FALSE(concepts::event_handler<missing_handler_for_single_event>);
}

TEST_CASE("handles specified event is an event handler for that event",
          "[unit][event_handler]") {
  REQUIRE(is_event_handler_for<missing_handler_for_single_event,
                               test_objects::test_event<0>>::value);
  REQUIRE(concepts::event_handler_for<missing_handler_for_single_event,
                                      test_objects::test_event<0>>);
}

TEST_CASE("valid event handler is an event handler", "[unit][event_handler]") {
  REQUIRE(is_event_handler<valid_event_handler>::value);
  REQUIRE(concepts::event_handler<valid_event_handler>);
}