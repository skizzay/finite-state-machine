#include "skizzay/fsm/state.h"

#include "test_objects.h"
#include <catch2/catch.hpp>

using namespace skizzay::fsm;

namespace {
struct indestructible final {
  ~indestructible() = delete;
};

struct copy_constructible final {
  copy_constructible(copy_constructible const &) = default;
};

struct move_constructible final {
  move_constructible(move_constructible &&) = default;
};

struct default_constructible final {
  default_constructible() = default;
  default_constructible(default_constructible const &) = delete;
  default_constructible(default_constructible &&) = delete;
};
} // namespace

TEST_CASE("void * is not a state", "[unit][state]") {
  REQUIRE_FALSE(concepts::state<void *>);
}

TEST_CASE("void is not a state", "[unit][state]") {
  REQUIRE_FALSE(concepts::state<void>);
}

TEST_CASE("indestructible is not a state", "[unit][state]") {
  REQUIRE_FALSE(concepts::state<indestructible>);
}

TEST_CASE("nonmovable/noncopyable constructible is not a state",
          "[unit][state]") {
  REQUIRE_FALSE(concepts::state<default_constructible>);
}

TEST_CASE("moveable constructible is a state", "[unit][state]") {
  REQUIRE_FALSE(concepts::state<move_constructible>);
}

TEST_CASE("copyable constructible is a state", "[unit][state]") {
  REQUIRE(concepts::state<copy_constructible>);
}

TEST_CASE("test state is a state", "[unit][state]") {
  REQUIRE(concepts::state<test_objects::test_state<0, 1>>);
}