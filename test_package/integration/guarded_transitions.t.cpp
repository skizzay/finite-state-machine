#include "skizzay/fsm/transition_table.h"
#include "skizzay/fsm/dispatcher.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/transition.h"
#include <skizzay/utilz/overload.h>
#include <skizzay/utilz/traits.h>
#include <catch.hpp>
#include <variant>

using namespace skizzay::fsm;

namespace {

using timer_expired = event<skizzay::utilz::tag<struct timer_expired_tag>>;

struct green {};
struct yellow {};
struct red {};

}

SCENARIO("guarded transition table", "[unit][variant]") {
   using light = state_container<std::variant<red, yellow, green>>;

   GIVEN("red") {
      light current_state{red{}};
      bool flag = true;
      auto guard1 = skizzay::utilz::overload{
         [&flag](red const &, timer_expired const &) { return flag; },
         [&flag](yellow const &, timer_expired const &) { return flag; },
         [&flag](green const &, timer_expired const &) { return flag; }
      };
      auto guard2 = std::not_fn(guard1);
      transition_table target{
         dispatcher{},
         make_guarded_transition<red, green, timer_expired>(guard1),
         make_guarded_transition<green, yellow, timer_expired>(guard1),
         make_guarded_transition<yellow, red, timer_expired>(guard2)
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
      auto guard1 = skizzay::utilz::overload{
         [&flag](red const &, timer_expired const &) { return flag; },
         [&flag](yellow const &, timer_expired const &) { return flag; },
         [&flag](green const &, timer_expired const &) { return flag; }
      };
      auto guard2 = std::not_fn(guard1);
      transition_table target{
         dispatcher{},
         make_guarded_transition<red, green, timer_expired>(guard1),
         make_guarded_transition<green, yellow, timer_expired>(guard1),
         make_guarded_transition<yellow, red, timer_expired>(guard2)
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
