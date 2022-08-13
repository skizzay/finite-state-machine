#pragma once

#include "skizzay/fsm/boolean.h"
#include "skizzay/fsm/const_ref.h"
#include "skizzay/fsm/states_list.h"

#include <type_traits>

namespace skizzay::fsm {

namespace no_op_details_ {
struct impl final {
  constexpr explicit impl(bool const is_done) noexcept : is_done_{is_done} {}
  constexpr bool is_done() const noexcept { return is_done_; }
  constexpr void operator()(auto const &...) const noexcept {}

private:
  bool is_done_;
};

struct impl_factory final {
  constexpr impl operator()(bool const is_done = true) const noexcept {
    return impl{is_done};
  }
};
} // namespace no_op_details_

inline namespace no_op_fn_ {
inline constexpr no_op_details_::impl_factory no_op = {};
}

namespace query_result_details_ {
template <typename...> struct as_states_list;

template <concepts::state... States>
struct as_states_list<states_list<States...>> {
  using type = states_list<States...>;
};

template <concepts::state... States> struct as_states_list<States...> {
  using type = states_list<States...>;
};
} // namespace query_result_details_

template <typename, typename...> struct query_result;
template <typename F, typename... T>
using query_result_t = typename query_result<F, T...>::type;

template <typename F, concepts::state... States>
struct query_result<F, states_list<States...>> : query_result<F, States...> {};

template <typename F, concepts::state... States>
struct query_result<F, States...>
    : std::common_type<typename query_result<F, States>::type...> {};

template <typename F, concepts::state State>
struct query_result<F, State> : std::invoke_result<F, add_cref_t<State>> {};

namespace concepts {
template <typename F, typename... States>
concept query = requires {
  typename query_result_t<F, States...>;
}
&&all_v<typename query_result_details_::as_states_list<States...>::type,
        curry<std::is_invocable, F>::template type>;

template <typename F, typename... States>
concept nothrow_query = query<F, States...> &&
    all_v<typename query_result_details_::as_states_list<States...>::type,
          curry<std::is_nothrow_invocable, F>::template type>;
} // namespace concepts

} // namespace skizzay::fsm
