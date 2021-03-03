#ifndef CRAB_ASSET_HPP
#define CRAB_ASSET_HPP
#include <string>

#include "currency_pair.hpp"

namespace crab {

/// Specific currency asset on a specific exchange/market.
struct Asset {
    std::string exchange;
    Currency_pair currency;
};

[[nodiscard]] inline auto operator==(Asset const& a, Asset const& b) -> bool
{
    return a.currency == b.currency && a.exchange == b.exchange;
}

[[nodiscard]] inline auto operator!=(Asset const& a, Asset const& b) -> bool
{
    return !(a == b);
}

// Total ordering for sort/unique/map
[[nodiscard]] inline auto operator<(Asset const& a, Asset const& b) -> bool
{
    if (a.exchange == b.exchange) {
        if (a.currency.base == b.currency.base) {
            return a.currency.quote < b.currency.quote;
        }
        return a.currency.base < b.currency.base;
    }
    return a.exchange < b.exchange;
}

}  // namespace crab
#endif  // CRAB_ASSET_HPP
