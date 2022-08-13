#pragma once

#include <type_traits>

namespace skizzay::fsm {

struct nonesuch final {
  nonesuch() = delete;
  nonesuch(nonesuch const &) = delete;
  nonesuch(nonesuch &&) = delete;
  ~nonesuch() = delete;

  void operator=(nonesuch const &) = delete;
  void operator=(nonesuch &&) = delete;
};

namespace is_detected_details_ {
template <typename Default, typename AlwaysVoid,
          template <typename...> typename, typename...>
struct impl {
  using type = Default;
  using value_type = std::false_type;
};

template <typename Default, template <typename...> typename Template,
          typename... Args>
struct impl<Default, std::void_t<Template<Args...>>, Template, Args...> {
  using type = Template<Args...>;
  using value_type = std::true_type;
};
} // namespace is_detected_details_

template <template <typename...> typename Template, typename... Ts>
struct is_detected
    : is_detected_details_::impl<nonesuch, void, Template, Ts...>::value_type {
};

template <template <typename...> typename Template, typename... Ts>
constexpr inline bool is_detected_v = is_detected<Template, Ts...>::value;

template <template <typename...> typename Template, typename... Ts>
struct detected {
  using type = typename is_detected_details_::impl<nonesuch, void, Template,
                                                   Ts...>::type;
};

template <template <typename...> typename Template, typename... Ts>
using detected_t = typename detected<Template, Ts...>::type;

template <typename Default, template <typename...> typename Template,
          typename... Ts>
struct detected_or {
  using type =
      typename is_detected_details_::impl<Default, void, Template, Ts...>::type;
};

template <typename Default, template <typename...> typename Template,
          typename... Ts>
using detected_or_t = typename detected_or<Default, Template, Ts...>::type;

} // namespace skizzay::fsm
