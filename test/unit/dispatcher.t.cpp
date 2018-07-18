#include "fsm/dispatcher.h"
#include "fsm/event.h"
#include <catch.hpp>

using namespace skizzay::fsm;

namespace {

struct fake_data {};
using fake_event = event<fake_data>;

struct fake_state {};

struct fake_transition {
   using current_state_type = fake_state;
   using next_state_type = fake_state;
   using event_type = fake_event;

   bool accept_value;
   mutable std::size_t accepts_invocations = 0;

   constexpr bool accepts(current_state_type const &, event_type const &) const noexcept {
      ++accepts_invocations;
      return accept_value;
   }
};

struct noop_handler {
   constexpr void operator()(fake_state const &, fake_event const &) const noexcept {
   }
};

template<class AmbiguityHandler, class UnfoundedHandler>
struct handler {
   AmbiguityHandler ambiguity_handler;
   UnfoundedHandler unfounded_handler;

   constexpr std::enable_if_t<!std::is_same_v<AmbiguityHandler, noop_handler>> on_ambiguous_trigger_found(fake_state const &s, fake_event const &e) {
      ambiguity_handler(s, e);
   }

   constexpr std::enable_if_t<!std::is_same_v<UnfoundedHandler, noop_handler>> on_no_trigger_found(fake_state const &s, fake_event const &e) {
      unfounded_handler(s, e);
   }
};

template <class T, class U> handler(T &&, U &&)-> handler<std::remove_reference_t<T>, std::remove_reference_t<U>>;
}

TEST_CASE("dispatcher", "[unit]") {
   std::size_t ambiguity_count = 0;
   std::size_t unfounded_count = 0;

   auto count_ambiguities = [&ambiguity_count](fake_state const &, fake_event const &) { ++ambiguity_count; };
   auto count_unfounded = [&unfounded_count](fake_state const &, fake_event const &) { ++unfounded_count; };

   SECTION("empty dispatcher") {
      dispatcher target{};

      SECTION("does not trigger on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(0 == unfounded_count);
      }

      SECTION("does not trigger on ambiguous trigger") {
         std::tuple transitions{fake_transition{false}};
         target.on_accepted(fake_state{}, event{fake_data{}}, transitions, std::index_sequence<0>{});

         REQUIRE(0 == ambiguity_count);
      }
   }

   SECTION("dispatcher with ambiguity handler") {
      dispatcher target{handler{std::move(count_ambiguities), noop_handler{}}};

      SECTION("does not trigger on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(0 == unfounded_count);
      }

      SECTION("does not trigger ambiguity handler when no transitions handle event") {
         std::tuple transitions{fake_transition{false}};
         target.on_accepted(fake_state{}, event{fake_data{}}, transitions, std::index_sequence<0>{});

         REQUIRE(0 == ambiguity_count);
      }

      SECTION("triggers ambiguity handler when a transition handles event") {
         std::tuple transitions{fake_transition{true}};
         target.on_accepted(fake_state{}, event{fake_data{}}, transitions, std::index_sequence<0>{});

         REQUIRE(1 == ambiguity_count);
      }
   }

   SECTION("dispatcher with unfounded handler") {
      dispatcher target{handler{noop_handler{}, std::move(count_unfounded)}};

      SECTION("triggers on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(1 == unfounded_count);
      }

      SECTION("does not trigger ambiguity handler when no transitions handle event") {
         std::tuple transitions{fake_transition{false}};
         target.on_accepted(fake_state{}, event{fake_data{}}, transitions, std::index_sequence<0>{});

         REQUIRE(0 == ambiguity_count);
      }
   }

   SECTION("dispatcher with unfounded and ambiguity handler in same handler") {
      dispatcher target{handler{std::move(count_ambiguities), std::move(count_unfounded)}};

      SECTION("triggers on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(1 == unfounded_count);
      }

      SECTION("triggers on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(1 == unfounded_count);
      }
   }

   SECTION("dispatcher with unfounded and ambiguity handler in separate handlers") {
      dispatcher target{handler{std::move(count_ambiguities), noop_handler{}},
                        handler{noop_handler{}, std::move(count_unfounded)}};

      SECTION("triggers on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(1 == unfounded_count);
      }

      SECTION("triggers on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(1 == unfounded_count);
      }
   }

   SECTION("dispatcher with unfounded and ambiguity handler in both same and separate handlers") {
      dispatcher target{handler{count_ambiguities, noop_handler{}},
                        handler{noop_handler{}, count_unfounded},
                        handler{count_ambiguities, count_unfounded}};

      SECTION("triggers on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(2 == unfounded_count);
      }

      SECTION("triggers on unfound transition") {
         target.on_no_trigger_found(fake_state{}, event{fake_data{}});

         REQUIRE(2 == unfounded_count);
      }
   }
}
