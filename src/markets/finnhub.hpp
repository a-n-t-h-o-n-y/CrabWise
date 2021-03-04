#ifndef CRAB_MARKETS_FINNHUB_HPP
#define CRAB_MARKETS_FINNHUB_HPP
#include <cassert>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <ntwk/check_response.hpp>
#include <ntwk/https_socket.hpp>
#include <ntwk/websocket.hpp>

#include "../asset.hpp"
#include "../currency_pair.hpp"
#include "../price.hpp"
#include "../search_result.hpp"
#include "../stats.hpp"
#include "error.hpp"
#include "to_ptree.hpp"

namespace crab {

/// Return the Finnhub 'symbol_id' cooresponding to the given Asset.
[[nodiscard]] auto find_symbol_id(Asset const& asset) -> std::string;

/// Return the Asset cooresponding to the given Finnhub 'symbol_id'.
[[nodiscard]] auto find_asset(std::string const& symbol_id) -> Asset;

class Finnhub {
   public:
    /// Connect internal https socket.
    void make_https_connection() { https_socket_.connect("finnhub.io"); }

    /// Disconnect internal https socket.
    void disconnect_https() { https_socket_.disconnect(); }

    /// Disconnect internal Websocket.
    void disconnect_websocket() { ws_.disconnect(); }

   public:
    /// Current and Opening price of Stock or Crypto
    [[nodiscard]] auto stats(Asset const& asset) -> Stats
    {
        auto const asset_str = find_symbol_id(asset);
        if (!https_socket_.is_connected())
            this->make_https_connection();
        auto const message =
            https_socket_.get(build_rest_query("quote?symbol=" + asset_str));
        check_response(message, "Finnhub - Failed to get stats");

        auto const tree = to_ptree(message.body);
        return {tree.get<std::string>("c"), tree.get<std::string>("o")};
    }

    /// Subscribe to websocket update for \p asset.
    void subscribe(Asset const& asset)
    {
        using ptree = boost::property_tree::ptree;
        auto tree   = ptree{};
        tree.add("type", "subscribe");
        tree.add("symbol", find_symbol_id(asset));
        auto ss = std::ostringstream{};
        write_json(ss, tree, false);
        if (!ws_.is_connected())
            this->ws_connect();
        ws_.write(ss.str());
        ++subscription_count_;
    }

    /// Subscribe to websocket update for \p asset.
    void unsubscribe(Asset const& asset)
    {
        using ptree = boost::property_tree::ptree;
        auto tree   = ptree{};
        tree.add("type", "unsubscribe");
        tree.add("symbol", find_symbol_id(asset));
        auto ss = std::ostringstream{};
        write_json(ss, tree, false);
        if (!ws_.is_connected())
            this->ws_connect();
        ws_.write(ss.str());
        --subscription_count_;
    }

    /// Read a single response from the websocket, parsed into multiple Prices
    auto stream_read() -> std::vector<Price>
    {
        auto prices = parse_prices(to_ptree(ws_.read()));
        // Remove Duplicates so up/down indicators work properly. Keep newest.
        std::stable_sort(
            std::begin(prices), std::end(prices),
            [](auto const& a, auto const& b) { return a.asset < b.asset; });
        auto const end = std::unique(
            std::rbegin(prices), std::rend(prices),
            [](auto const& a, auto const& b) { return a.asset == b.asset; });
        prices.erase(std::begin(prices), end.base());
        return prices;
    }

    /// Return number of subscriptions on the websocket.
    [[nodiscard]] auto subscription_count() const
    {
        return subscription_count_;
    }

    [[nodiscard]] auto search(std::string const& query)
        -> std::vector<Search_result>
    {
        if (!https_socket_.is_connected())
            this->make_https_connection();
        auto const message =
            https_socket_.get(build_rest_query("search?q=" + query));
        check_response(message, "Finnhub - Failed to search for: " + query);

        auto result        = std::vector<Search_result>{};
        auto const results = to_ptree(message.body).get_child("result");
        for (auto const& x : results) {
            auto const description = x.second.get<std::string>("description");
            auto const symbol_id   = x.second.get<std::string>("symbol");
            auto const type        = x.second.get<std::string>("type");
            auto sr                = Search_result{type, description, {}};
            if (type == "Crypto") {
                sr.asset = find_asset(symbol_id);
                result.push_back(sr);
            }
            if (type == "Common Stock") {
                if (symbol_id.find('.') != std::string::npos)
                    continue;  // Foreign Exchange
                sr.asset = Asset{"", {symbol_id, "USD"}};
                result.push_back(sr);
            }
        }
        return result;
    }

   private:
    ntwk::Websocket ws_;
    mutable ntwk::HTTPS_socket https_socket_;
    std::string key_param_;
    int subscription_count_ = 0;

   private:
    [[nodiscard]] auto build_rest_query(std::string resource) -> std::string
    {
        if (!resource.empty() && (resource.find('?') == std::string::npos))
            resource.push_back('?');
        else
            resource.push_back('&');
        return "/api/v1/" + resource + this->get_key();
    }

    /// Build ptree array with this.
    static void push_back(boost::property_tree::ptree& array,
                          std::string const& element)
    {
        auto foo = boost::property_tree::ptree{};
        foo.put("", element);
        array.push_back(std::make_pair("", std::move(foo)));
    }

    [[nodiscard]] auto parse_prices(boost::property_tree::ptree const& tree)
        -> std::vector<Price>
    {
        if (tree.get<std::string>("type") != "trade")
            return {};

        auto const array = tree.get_child("data");
        auto result      = std::vector<Price>{};
        for (auto const& item : array) {
            auto const symbol_id = item.second.get<std::string>("s");
            result.push_back(
                {item.second.get<std::string>("p"), find_asset(symbol_id)});
        }
        return result;
    }

    auto get_key() -> std::string const&
    {
        if (key_param_.empty())
            key_param_ = "token=" + parse_key("finnhub.key");
        return key_param_;
    }

    void ws_connect() { ws_.connect("ws.finnhub.io", "/?" + this->get_key()); }

    [[nodiscard]] static auto parse_key(std::string const& filename)
        -> std::string
    {
        auto file = std::ifstream{filename};
        auto key  = std::string{};
        file >> key;
        return key;
    }
};

}  // namespace crab
#endif  // CRAB_MARKETS_FINNHUB_HPP
