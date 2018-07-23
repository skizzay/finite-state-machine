#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/dispatcher.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/utils/overload.h"
#include <catch.hpp>
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
         make_simple_transition<red, green, timer_expired>(),
         make_simple_transition<green, yellow, timer_expired>(),
         make_simple_transition<yellow, red, timer_expired>()
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
         make_simple_transition<red, green, timer_expired>(),
         make_simple_transition<green, yellow, timer_expired>(),
         make_simple_transition<yellow, red, timer_expired>()
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
