#include "skizzay/fsm/traits.h"
#include "skizzay/fsm/event.h"
#include "skizzay/fsm/transition.h"
#include <skizzay/utilz/detected.h>
#include <skizzay/utilz/traits.h>
#include <catch.hpp>
#include <variant>

using namespace skizzay::fsm;

namespace {

struct state_for_transition_traits {};
struct event_for_transition_traits : event<> {};

struct missing_current_state_type {
   using next_state_type = state_for_transition_traits;
   using event_type = event_for_transition_traits;

   template<class State>
   bool accepts(State const &, event_type const &);
};

struct missing_next_state_type {
   using current_state_type = state_for_transition_traits;
   using event_type = event_for_transition_traits;

   bool accepts(current_state_type const &, event_type const &);
};

struct missing_event_type {
   using current_state_type = state_for_transition_traits;
   using next_state_type = state_for_transition_traits;

   template<class Event>
   bool accepts(current_state_type const &, Event const &);
};

struct event_type_is_not_an_event {
   using current_state_type = state_for_transition_traits;
   using next_state_type = state_for_transition_traits;
   using event_type = skizzay::utilz::nonesuch;

   bool accepts(current_state_type const &, skizzay::utilz::nonesuch const &);
};

struct missing_compatible_accepts_method {
   using current_state_type = state_for_transition_traits;
   using next_state_type = state_for_transition_traits;
   using event_type = event_for_transition_traits;
};

using valid_transition = simple_transition<state_for_transition_traits,
      state_for_transition_traits, event_for_transition_traits>;
}

TEST_CASE("traits", "[unit]") {
   SECTION("event traits") {
      SECTION("tagged event") {
         struct dto {};
         using tagged_event = event<skizzay::utilz::tag<struct some_tag>, dto>;

         REQUIRE(is_event_v<tagged_event>);
      }

      SECTION("untagged event") {
         struct dto {};
         using untagged_event = event<dto>;

         REQUIRE(is_event_v<untagged_event>);
      }

      SECTION("inherited, tagged event") {
         struct dto {};
         struct inherited_tagged_event : event<skizzay::utilz::tag<struct some_tag>, dto> {};

         REQUIRE(is_event_v<inherited_tagged_event>);
      }

      SECTION("inherited, untagged event") {
         struct dto {};
         struct inherited_untagged_event : event<dto> {};

         REQUIRE(is_event_v<inherited_untagged_event>);
      }

      SECTION("non-event") {
         struct non_event {
            operator event<> const &() const;
         };

         REQUIRE_FALSE(is_event_v<non_event>);
      }
   }

   SECTION("transition traits") {
      SECTION("missing current_state_type") {
         REQUIRE_FALSE(is_transition_v<missing_current_state_type>);
      }

      SECTION("missing next_state_type") {
         REQUIRE_FALSE(is_transition_v<missing_next_state_type>);
      }

      SECTION("missing event_type") {
         REQUIRE_FALSE(is_transition_v<missing_event_type>);
      }

      SECTION("event_type is not an event") {
         REQUIRE_FALSE(is_transition_v<event_type_is_not_an_event>);
      }

      SECTION("missing compatible accepts method") {
         REQUIRE_FALSE(is_transition_v<missing_compatible_accepts_method>);
      }

      SECTION("valid transition") {
         REQUIRE(is_transition_v<valid_transition>);
      }
   }

   SECTION("variant traits") {
      SECTION("variant type") {
         struct dto {};
         using tagged_event = event<skizzay::utilz::tag<struct some_tag>, dto>;
         using untagged_event = event<dto>;
         using variant_type = std::variant<tagged_event, untagged_event>;

         REQUIRE(skizzay::utilz::is_variant_v<variant_type>);
      }

      SECTION("non-variant type") {
         struct dto {};
         REQUIRE_FALSE(skizzay::utilz::is_variant_v<dto>);
      }
   }
}
