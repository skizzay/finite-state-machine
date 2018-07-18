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

struct timer_expired : skizzay::fsm::event<> {};

struct green {};
struct yellow {};
struct red {};

}

SCENARIO("simple transition table", "[unit][variant]") {
   using light = state_container<std::variant<red, yellow, green>>;

   GIVEN("red") {
      light current_state{red{}};
      transition_table target{
         dispatcher{},
         simple_transition<red, green, timer_expired>{},
         simple_transition<green, yellow, timer_expired>{},
         simple_transition<yellow, red, timer_expired>{},
      };

      WHEN("timer expired") {
         current_state.dispatch(timer_expired{}, target);

         THEN("the current state is green") {
            REQUIRE(current_state.is<green>());
         }

         WHEN("timer expired") {
            current_state.dispatch(timer_expired{}, target);

            THEN("the current state is yellow") {
               REQUIRE(current_state.is<yellow>());
            }

            WHEN("timer expired") {
               current_state.dispatch(timer_expired{}, target);

               THEN("the current state is red") {
                  REQUIRE(current_state.is<red>());
               }
            }
         }
      }
   }

   GIVEN("green") {
      light current_state{green{}};
      transition_table target{
         dispatcher{},
         simple_transition<red, green, timer_expired>{},
         simple_transition<green, yellow, timer_expired>{},
         simple_transition<yellow, red, timer_expired>{},
      };

      WHEN("timer expired") {
         current_state.dispatch(timer_expired{}, target);

         THEN("the current state is yellow") {
            REQUIRE(current_state.is<yellow>());
         }

         WHEN("timer expired") {
            current_state.dispatch(timer_expired{}, target);

            THEN("the current state is red") {
               REQUIRE(current_state.is<red>());
            }

            WHEN("timer expired") {
               current_state.dispatch(timer_expired{}, target);

               THEN("the current state is green") {
                  REQUIRE(current_state.is<green>());
               }
            }
         }
      }
   }
}
