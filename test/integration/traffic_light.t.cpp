#include "skizzay/fsm/dispatcher.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/finite_state_machine.h"
#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/transition.h"
#include "skizzay/fsm/transition_table.h"
#include <catch.hpp>
#include <chrono>
#include <cstdint>
#include <variant>

using namespace skizzay::fsm;

namespace {
   using timestamp_t = std::chrono::system_clock::time_point;
   using event_id_t = std::int64_t;

   struct event_header {
      event_id_t event_id;
      timestamp_t timestamp;
   };

   template<class ...TagsAndEventData>
   using traffic_light_event = event<tag<struct traffic_light_event_tag>, event_header, TagsAndEventData...>;

   using timer_expired = traffic_light_event<tag<struct timer_expired_tag>>;

   enum class color {
      red,
      yellow,
      green
   };

   template<color C>
   struct light {
      constexpr static color light_color = C;
   };

   using red_light = light<color::red>;
   using yellow_light = light<color::yellow>;
   using green_light = light<color::green>;
}

SCENARIO("Traffic light", "[integration]") {
   auto make_timer_expired = []() noexcept {
      auto get_event_id = []() noexcept {
         event_id_t current{0};
         return [current]() mutable noexcept { return ++current; };
      };

      auto now = []() noexcept { return std::chrono::system_clock::now(); };

      return [get_event_id=get_event_id(), now=now]() mutable noexcept {
         return timer_expired{{get_event_id(), now()}};
      };
   }();

   GIVEN("a red light") {
      finite_state_machine target{
         transition_table{
            dispatcher{throw_on_no_trigger{}, throw_on_ambiguous_trigger{}},
            make_simple_transition<red_light, green_light, timer_expired>(),
            make_simple_transition<green_light, yellow_light, timer_expired>(),
            make_simple_transition<yellow_light, red_light, timer_expired>()
         },
         state_container<std::variant<red_light, yellow_light, green_light>>{std::in_place_type<red_light>}
      };

      WHEN("time has expired") {
         target.on(make_timer_expired());

         THEN("the light is green") {
            REQUIRE(target.is<green_light>());
         }

         AND_WHEN("time has expired") {
            target.on(make_timer_expired());

            THEN("the light is yellow") {
               REQUIRE(target.is<yellow_light>());
            }

            AND_WHEN("time has expired") {
               target.on(make_timer_expired());

               THEN("the light is red") {
                  REQUIRE(target.is<red_light>());
               }
            }
         }
      }
   }
}
