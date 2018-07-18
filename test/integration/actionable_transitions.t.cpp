#include "fsm/transition_table.h"
#include "fsm/dispatcher.h"
#include "fsm/event.h"
#include "fsm/overload.h"
#include "fsm/state_container.h"
#include "fsm/transition.h"
#include "catch.hpp"
#include <variant>

using namespace skizzay::fsm;

namespace {

using timer_expired = event<tag<struct timer_expired_tag>>;

struct green {};
struct yellow {};
struct red {};

}

SCENARIO("action transition table", "[unit][variant]") {
   using light = state_container<std::variant<red, yellow, green>>;

   GIVEN("red") {
      std::size_t transition_count = 0;
      light current_state{red{}};
      auto increment_transition_count = [&transition_count](timer_expired const &e[[maybe_unused]]) { ++transition_count; };
      transition_table target{
         dispatcher{},
         action_transition<red, green, timer_expired, decltype(increment_transition_count)>{increment_transition_count},
         action_transition<green, yellow, timer_expired, decltype(increment_transition_count)>{increment_transition_count},
         action_transition<yellow, red, timer_expired, decltype(increment_transition_count)>{increment_transition_count}
      };

      WHEN("timer expired") {
         current_state.dispatch(timer_expired{}, target);

         THEN("the current state is green") {
            REQUIRE(current_state.is<green>());

            AND_THEN("transition count incremented") {
               REQUIRE(1 == transition_count);
            }
         }

         WHEN("timer expired") {
            current_state.dispatch(timer_expired{}, target);

            THEN("the current state is yellow") {
               REQUIRE(current_state.is<yellow>());

               AND_THEN("transition count incremented") {
                  REQUIRE(2 == transition_count);
               }
            }

            WHEN("timer expired") {
               current_state.dispatch(timer_expired{}, target);

               THEN("the current state is red") {
                  REQUIRE(current_state.is<red>());

                  AND_THEN("transition count incremented") {
                     REQUIRE(3 == transition_count);
                  }
               }
            }
         }
      }
   }

   GIVEN("green") {
      std::size_t transition_count = 0;
      light current_state{green{}};
      auto increment_transition_count = [&transition_count](timer_expired const &e[[maybe_unused]]) { ++transition_count; };
      transition_table target{
         dispatcher{},
         action_transition<red, green, timer_expired, decltype(increment_transition_count)>{increment_transition_count},
         action_transition<green, yellow, timer_expired, decltype(increment_transition_count)>{increment_transition_count},
         action_transition<yellow, red, timer_expired, decltype(increment_transition_count)>{increment_transition_count}
      };

      WHEN("timer expired") {
         current_state.dispatch(timer_expired{}, target);

         THEN("the current state is yellow") {
            REQUIRE(current_state.is<yellow>());

            AND_THEN("transition count incremented") {
               REQUIRE(1 == transition_count);
            }
         }

         WHEN("timer expired") {
            current_state.dispatch(timer_expired{}, target);

            THEN("the current state is red") {
               REQUIRE(current_state.is<red>());

               AND_THEN("transition count incremented") {
                  REQUIRE(2 == transition_count);
               }
            }

            WHEN("timer expired") {
               current_state.dispatch(timer_expired{}, target);

               THEN("the current state is green") {
                  REQUIRE(current_state.is<green>());

                  AND_THEN("transition count incremented") {
                     REQUIRE(3 == transition_count);
                  }
               }
            }
         }
      }
   }
}
