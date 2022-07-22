#include "skizzay/fsm/event_dispatcher.h"

#include "test_objects.h"
#include <catch2/catch.hpp>
#include <iostream>
#include <skizzay/fsm/action_transition.h>
#include <skizzay/fsm/simple_transition.h>
#include <skizzay/fsm/single_state_container.h>
#include <tuple>

using namespace skizzay::fsm;

constexpr std::size_t num_events = 1;
using state_type = test_objects::test_state<0, num_events>;
using external_state_type = test_objects::test_state<1, num_events>;

SCENARIO("Event dispatcher concepts") {
  using transition_table_type = std::tuple<
      simple_transition<state_type, state_type, test_objects::test_event<0>>>;
  using target_type =
      event_dispatcher<transition_table_type, single_state<state_type>>;
  REQUIRE(concepts::transition_table<transition_table_t<target_type>>);
  REQUIRE(concepts::events_list<events_list_t<target_type>>);
  REQUIRE(1 == length_v<transition_table_t<target_type>>);
  REQUIRE(1 == length_v<events_list_t<transition_table_t<target_type>>>);
  REQUIRE(num_events == length_v<events_list_t<target_type>>);
  REQUIRE(concepts::event_engine_for<target_type, test_objects::test_event<0>>);
  REQUIRE(event_engine_details_::has_post_event_template_member_function<
          target_type, test_objects::test_event<0>>::value);
  REQUIRE(all_v<
          events_list_t<target_type>,
          curry<event_engine_details_::has_post_event_template_member_function,
                target_type>::template type>);
  REQUIRE(concepts::event_engine<target_type>);
}

SCENARIO("Event dispatching", "[unit][event-dispatcher]") {
  single_state<state_type> container;
  test_objects::fake_entry_context<initial_entry_event_t,
                                   states_list<state_type>,
                                   test_objects::test_events_list<num_events>>
      initial_entry_context;
  execute_initial_entry(container, initial_entry_context,
                        initial_entry_context);

  GIVEN("an event dispatcher") {
    constexpr std::size_t const event_id = 0;
    using event_type = test_objects::test_event<event_id>;
    std::tuple transition_table{
        simple_transition<state_type, state_type, event_type>{},
        simple_transition<state_type, state_type, epsilon_event_t>{}};
    REQUIRE_FALSE(empty_v<decltype(get_transition_table_for_current_state(
                      transition_table, event_type{}, state_type{}))>);
    event_dispatcher target{std::move(transition_table), container};
    WHEN("a handled event is dispatched") {
      bool const handled = target.dispatch_event(event_type{});

      THEN("it has been handled") {
        REQUIRE(handled);
        AND_THEN("the state was reentered") {
          REQUIRE(1 == container.current_state<state_type>()
                           .value()
                           .event_reentry_count[event_id]);
        }
      }
    }

    WHEN("an unhandled event is dispatched") {
      bool const handled = target.dispatch_event(event_type{false});

      THEN("it has not been handled") { REQUIRE_FALSE(handled); }
    }
  }
}

SCENARIO("Event posting", "[unit][event-dispatcher]") {
  single_state<state_type> container;
  test_objects::fake_entry_context<initial_entry_event_t,
                                   states_list<state_type>,
                                   test_objects::test_events_list<num_events>>
      initial_entry_context;
  execute_initial_entry(container, initial_entry_context,
                        initial_entry_context);

  GIVEN("an event dispatcher") {
    constexpr std::size_t const event_id = 0;
    using event_type = test_objects::test_event<event_id>;
    std::tuple transition_table{
        simple_transition<state_type, state_type, epsilon_event_t>{},
        simple_transition<state_type, state_type, event_type>{}};
    event_dispatcher target{std::move(transition_table), container};

    WHEN("a handled event is dispatched") {
      bool const handled = target.dispatch_event(event_type{});

      THEN("it has been handled") {
        REQUIRE(handled);
      }
    }

    WHEN("an unhandled event is dispatched") {
      bool const handled = target.dispatch_event(event_type{false});

      THEN("it has not been handled") { REQUIRE_FALSE(handled); }
    }
  }
}