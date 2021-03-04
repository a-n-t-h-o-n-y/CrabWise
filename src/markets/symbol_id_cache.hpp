#ifndef CRAB_MARKETS_SYMBOL_ID_CACHE_HPP
#define CRAB_MARKETS_SYMBOL_ID_CACHE_HPP
#include <string>

#include "../asset.hpp"

namespace crab {

/// Return the Finnhub 'symbol_id' cooresponding to the given Asset.
[[nodiscard]] auto find_symbol_id(Asset const& asset) -> std::string;

/// Return the Asset cooresponding to the given Finnhub 'symbol_id'.
[[nodiscard]] auto find_asset(std::string const& symbol_id) -> Asset;

}  // namespace crab
#endif  // CRAB_MARKETS_SYMBOL_ID_CACHE_HPP
