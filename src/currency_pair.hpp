#ifndef CRAB_CURRENCY_PAIR_HPP
#define CRAB_CURRENCY_PAIR_HPP
#include <string>

namespace crab {

/// Base/Quote Currency Pair
struct Currency_pair {
    std::string base;
    std::string quote;
};

[[nodiscard]] inline auto operator==(Currency_pair const& a,
                                     Currency_pair const& b) -> bool
{
    return a.base == b.base && a.quote == b.quote;
}

[[nodiscard]] inline auto operator!=(Currency_pair const& a,
                                     Currency_pair const& b) -> bool
{
    return !(a == b);
}

}  // namespace crab
#endif  // CRAB_CURRENCY_PAIR_HPP
