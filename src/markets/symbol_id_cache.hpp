#ifndef CRAB_MARKETS_SYMBOL_ID_CACHE_HPP
#define CRAB_MARKETS_SYMBOL_ID_CACHE_HPP
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "../asset.hpp"

namespace crab {

class Symbol_ID_cache {
   public:
    Symbol_ID_cache(std::vector<std::pair<std::string, Asset>> cache)
    {
        for (auto const& [id, asset] : cache)
            get_asset_map_[id] = asset;
        for (auto const& [id, asset] : cache)
            get_id_map_[asset] = id;
    }

   public:
    /// Return the Finnhub 'symbol_id' cooresponding to the given Asset.
    [[nodiscard]] auto find_symbol_id(Asset const& asset) -> std::string
    {
        // Stocks are not in map.
        if (get_id_map_.count(asset) == 0)
            return asset.currency.base;
        else
            return get_id_map_[asset];
    }

    /// Return the Asset cooresponding to the given Finnhub 'symbol_id'.
    [[nodiscard]] auto find_asset(std::string const& symbol_id) -> Asset
    {
        // Stocks are not in map.
        if (get_asset_map_.count(symbol_id) == 0)
            return {"", {symbol_id, "USD"}};
        else
            return get_asset_map_[symbol_id];
    }

    /// Return whether or not the symbol_id is cached.
    [[nodiscard]] auto is_cached(std::string const& symbol_id) const -> bool
    {
        return get_asset_map_.count(symbol_id) == 1;
    }

   private:
    // [key: symbol_id, value: asset]
    std::map<std::string, Asset> get_asset_map_;

    // [key: Asset, value: symbol_id]
    std::map<Asset, std::string> get_id_map_;
};

}  // namespace crab
#endif  // CRAB_MARKETS_SYMBOL_ID_CACHE_HPP
