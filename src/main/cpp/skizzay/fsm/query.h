#pragma once

#include "skizzay/fsm/boolean.h"
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
template <typename> struct has_states_list_t_impl : std::false_type {};
template <typename T>
requires requires { typename states_list_t<T>; }
struct has_states_list_t_impl<T> : std::true_type {};

template <typename T>
concept has_states_list_t = has_states_list_t_impl<T>::value;
} // namespace query_result_details_

template <typename, typename> struct query_result;
template <typename F, typename T>
using query_result_t = typename query_result<F, T>::type;

template <typename F, template <typename...> typename Template,
          concepts::state... States>
struct query_result<F, Template<States...>> {
  using type = std::common_type_t<typename query_result<F, States>::type...>;
};

template <typename F, query_result_details_::has_states_list_t HasStatesList>
struct query_result<F, HasStatesList>
    : query_result<F, states_list_t<HasStatesList>> {};

template <typename F, concepts::state State>
requires(!query_result_details_::has_states_list_t<State>) struct query_result<
    F, State> : std::invoke_result<F, add_cref_t<State>> {
};

namespace concepts {
template <typename F, typename StatesList>
concept query = states_list<StatesList> &&
    all_v<StatesList, curry<std::is_invocable, F>::template type>;

template <typename F, typename StatesList>
concept nothrow_query = query<F, StatesList> &&
    all_v<StatesList, curry<std::is_nothrow_invocable, F>::template type>;

template <typename F, typename StatesList>
concept completing_query = query<F, StatesList> && requires(F const &fc) {
  { fc.is_done() }
  noexcept->boolean;
};
} // namespace concepts

} // namespace skizzay::fsm
