#include <catch.hpp>
#include <skizzay/fsm/enter.h>
#include <skizzay/fsm/event.h>

using skizzay::fsm::epsilon_event;
using skizzay::fsm::epsilon_event_t;

namespace {

struct simple_state {
  std::size_t entry_count = 0;
};

struct adl_state {
  std::size_t entry_count = 0;
};

void on_entry(adl_state &s, epsilon_event_t const &) noexcept {
  ++s.entry_count;
}

struct member_function_state {
  std::size_t entry_count = 0;

  void on_entry(epsilon_event_t const &) noexcept { ++entry_count; }
};

struct child_state {
  std::size_t entry_count = 0;

  template <typename ParentStates>
  void on_entry(auto const &, ParentStates parents) {
    ++entry_count;
    std::apply([](auto &...parents) noexcept { (++parents.entry_count, ...); },
               parents);
  }
};

} // namespace

SCENARIO("enter callbacks", "[unit][state]") {
  GIVEN("a state without any callbacks") {
    simple_state target;

    WHEN("entered") {
      skizzay::fsm::enter(target, epsilon_event, std::tuple{});
      THEN("entry callback was not fired") { REQUIRE(0 == target.entry_count); }
    }
  }

  GIVEN("a state with ADL callbacks") {
    adl_state target;

    WHEN("entered") {
      skizzay::fsm::enter(target, epsilon_event, std::tuple{});
      THEN("entry callback was fired") { REQUIRE(1 == target.entry_count); }
    }
  }

  GIVEN("a state with member function callbacks") {
    member_function_state target;

    WHEN("entered") {
      skizzay::fsm::enter(target, epsilon_event, std::tuple{});
      THEN("entry callback was fired") { REQUIRE(1 == target.entry_count); }
    }
  }

  GIVEN("a state with parental callbacks") {
    simple_state parent;
    child_state child;

    AND_GIVEN("a grandparent") {
      member_function_state grandparent;

      WHEN("entered") {
        auto target = std::tie(parent, grandparent);
        skizzay::fsm::enter(child, epsilon_event, target);
        THEN("child entry callback was fired") {
          REQUIRE(1 == child.entry_count);
        }
        THEN("parent entry callback was fired") {
          REQUIRE(1 == parent.entry_count);
        }
        THEN("grandparent entry callback was fired") {
          REQUIRE(1 == grandparent.entry_count);
        }
      }
    }

    WHEN("entered") {
      skizzay::fsm::enter(child, epsilon_event, std::tie(parent));
      THEN("child entry callback was fired") {
        REQUIRE(1 == child.entry_count);
      }
      THEN("parent entry callback was fired") {
        REQUIRE(1 == parent.entry_count);
      }
    }
  }
}