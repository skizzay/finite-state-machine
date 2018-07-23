#pragma once

#include <type_traits>

namespace skizzay::utils {

template<class ...Fs> struct overload : Fs... { using Fs::operator()...; };
template<class ...Fs> overload(Fs &&...) -> overload<std::remove_const_t<std::remove_reference_t<Fs>>...>;

}
