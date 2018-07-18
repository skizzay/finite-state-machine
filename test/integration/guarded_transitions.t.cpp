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

SCENARIO("guarded transition table", "[unit][variant]") {
   using light = state_container<std::variant<red, yellow, green>>;

   GIVEN("red") {
      light current_state{red{}};
      bool flag = true;
      auto guard1 = overload{
         [&flag](red const &s, timer_expired const &e) { return flag; },
         [&flag](yellow const &s, timer_expired const &e) { return flag; },
         [&flag](green const &s, timer_expired const &e) { return flag; }
      };
      auto guard2 = std::not_fn(guard1);
      transition_table target{
         dispatcher{},
         guarded_transition<red, green, timer_expired, decltype(guard1)>{guard1},
         guarded_transition<green, yellow, timer_expired, decltype(guard1)>{guard1},
         guarded_transition<yellow, red, timer_expired, decltype(guard2)>{guard2}
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

               THEN("the current state is yellow") {
                  REQUIRE(current_state.is<yellow>());
               }
            }
         }
      }
   }

   GIVEN("green") {
      light current_state{green{}};
      bool flag = true;
      auto guard1 = overload{
         [&flag](red const &s, timer_expired const &e) { return flag; },
         [&flag](yellow const &s, timer_expired const &e) { return flag; },
         [&flag](green const &s, timer_expired const &e) { return flag; }
      };
      auto guard2 = std::not_fn(guard1);
      transition_table target{
         dispatcher{},
         guarded_transition<red, green, timer_expired, decltype(guard1)>{guard1},
         guarded_transition<green, yellow, timer_expired, decltype(guard1)>{guard1},
         guarded_transition<yellow, red, timer_expired, decltype(guard2)>{guard2}
      };

      WHEN("timer expired") {
         current_state.dispatch(timer_expired{}, target);

         THEN("the current state is yellow") {
            REQUIRE(current_state.is<yellow>());
         }

         WHEN("timer expired") {
            current_state.dispatch(timer_expired{}, target);

            THEN("the current state is yellow") {
               REQUIRE(current_state.is<yellow>());
            }

            WHEN("the flag is flipped") {
               flag = !flag;

               AND_WHEN("timer expired") {
                  current_state.dispatch(timer_expired{}, target);

                  THEN("the current state is red") {
                     REQUIRE(current_state.is<red>());
                  }
               }
            }
         }
      }
   }
}
