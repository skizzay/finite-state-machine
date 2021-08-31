#pragma once

#include <type_traits>

namespace skizzay::fsm {
template <typename... Fs> struct overload : Fs... { using Fs::operator()...; };
template <typename... Fs>
overload(Fs &&...) -> overload<std::remove_cvref_t<Fs>...>;
} // namespace skizzay::fsm