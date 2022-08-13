#include <skizzay/fsm/enter.h>

#include "skizzay/fsm/event.h"
#include "skizzay/fsm/states_list.h"
#include "test_objects.h"
#include <catch.hpp>

using namespace skizzay::fsm;

namespace {

constexpr std::size_t num_events = 2;

struct simple_state : test_objects::test_state<0, num_events> {
  template <std::size_t EventId>
  void on_entry(test_objects::test_event<EventId> const &) = delete;
};

struct adl_state : test_objects::test_state<1, num_events> {
  template<std::size_t EventId>
  void on_entry(test_objects::test_event<EventId> const &) = delete;
};

void on_entry(adl_state &s, epsilon_event_t const &) noexcept {
  ++s.epsilon_event_entry_count;
}

using member_function_state = test_objects::test_state<2, num_events>;

using child_state = test_objects::test_state<3, num_events>;

} // namespace

SCENARIO("enter callbacks", "[unit][state]") {
  test_objects::fake_event_engine<test_objects::test_events_list<num_events>>
      event_engine;
  test_objects::fake_state_provider<
      states_list<simple_state, adl_state, member_function_state, child_state>>
      state_provider;

  GIVEN("a state without any callbacks") {
    simple_state &target = state_provider.state<simple_state>();

    WHEN("entered") {
      skizzay::fsm::enter(target, epsilon_event, event_engine, state_provider);
      THEN("entry callback was not fired") { REQUIRE(0 == target.epsilon_event_entry_count); }
    }
  }

  GIVEN("a state with ADL callbacks") {
    adl_state &target = state_provider.state<adl_state>();

    WHEN("entered") {
      skizzay::fsm::enter(target, epsilon_event, event_engine, state_provider);
      THEN("entry callback was fired") {
        REQUIRE(1 == target.epsilon_event_entry_count);
      }
    }
  }

  GIVEN("a state with member function callbacks") {
    member_function_state &target =
        state_provider.state<member_function_state>();

    WHEN("entered") {
      skizzay::fsm::enter(target, epsilon_event, event_engine, state_provider);
      THEN("entry callback was fired") {
        REQUIRE(1 == target.epsilon_event_entry_count);
      }
    }
  }
}