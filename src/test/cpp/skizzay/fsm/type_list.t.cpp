#include <catch.hpp>
#include <skizzay/fsm/type_list.h>
#include <tuple>
#include <type_traits>
#include <variant>

using namespace skizzay::fsm;
using target_type = std::tuple<int, float>;

TEST_CASE("all", "[type_list]") {
  REQUIRE(all_v<target_type, std::is_arithmetic>);
  REQUIRE_FALSE(all_v<target_type, std::is_integral>);
}

TEST_CASE("any", "[type_list]") {
  REQUIRE(any_v<target_type, std::is_integral>);
  REQUIRE_FALSE(any_v<target_type, std::is_union>);
}

TEST_CASE("none", "[type_list]") {
  REQUIRE_FALSE(none_v<target_type, std::is_integral>);
  REQUIRE(none_v<target_type, std::is_union>);
}

TEST_CASE("contains", "[type_list]") {
  REQUIRE(contains_v<target_type, int>);
  REQUIRE_FALSE(contains_v<target_type, double>);
  REQUIRE(contains_v<std::index_sequence<0, 1, 2>,
                     std::integral_constant<std::size_t, 0>>);
  REQUIRE_FALSE(contains_v<std::index_sequence<0, 1, 2>,
                           std::integral_constant<std::size_t, 8>>);
}

TEST_CASE("as_container", "[type_list]") {
  REQUIRE(std::is_same_v<as_container_t<target_type, std::variant>,
                         std::variant<int, float>>);
}

TEST_CASE("element_at", "[type_list]") {
  REQUIRE(std::is_same_v<element_at_t<1, target_type>, float>);
}

TEST_CASE("front", "[type_list]") {
  REQUIRE(std::is_same_v<front_t<target_type>, int>);
}

TEST_CASE("back", "[type_list]") {
  REQUIRE(std::is_same_v<back_t<target_type>, float>);
}