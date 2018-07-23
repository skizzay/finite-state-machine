#include "skizzay/fsm/state_container.h"
#include "skizzay/fsm/event.h"
#include "skizzay/utils/overload.h"
#include <catch.hpp>

using namespace skizzay::fsm;

namespace {

using fake_event = event<>;

struct state1 {};
struct state2 {};
struct state3 {};

}

TEST_CASE("state_container", "[unit]") {
   using target_type = state_container<std::variant<state1, state2, state3>>;

   target_type target{state1{}};

   SECTION("in initial state") {
      REQUIRE(target.template is<state1>());
      REQUIRE_FALSE(target.template is<state2>());
   }

   SECTION("construct") {
      decltype(auto) result = target.construct<state2>(state1{}, fake_event{});

      SECTION("in next state") {
         REQUIRE(target.template is<state2>());
         REQUIRE_FALSE(target.template is<state1>());
      }

      SECTION("returns reference to next state") {
         REQUIRE(std::is_same_v<state2,
                                std::remove_const_t<
                                    std::remove_reference_t<decltype(result)>>>);
      }
   }

   SECTION("query visits current state") {
      bool const result = target.query(skizzay::utils::overload{
            [](state1 const &) { return true; },
            [](auto const &) { return false; }
         });
      REQUIRE(result);
   }
}
