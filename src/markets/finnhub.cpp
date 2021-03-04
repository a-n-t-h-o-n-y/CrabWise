#include "finnhub.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <simdjson.h>

#include <ntwk/check_response.hpp>
#include <ntwk/https_socket.hpp>
#include <ntwk/websocket.hpp>

#include "../asset.hpp"
#include "../price.hpp"
#include "../stats.hpp"
#include "symbol_id_cache.hpp"

namespace {

/// Build an HTTP REST query string for finnhub from a resource and key.
[[nodiscard]] auto build_rest_query(std::string resource,
                                    std::string const& key) -> std::string
{
    if (!resource.empty() && (resource.find('?') == std::string::npos))
        resource.push_back('?');
    else
        resource.push_back('&');
    return "/api/v1/" + resource + key;
}

using JSON_element_t = simdjson::simdjson_result<simdjson::dom::element>;

[[nodiscard]] auto parse_prices(JSON_element_t const& e)
    -> std::vector<crab::Price>
{
    if ((std::string)e["type"] != "trade")
        return {};
    auto const array = e["data"];
    auto result      = std::vector<crab::Price>{};
    for (auto const& item : array) {
        auto const symbol_id = (std::string)item["s"];
        result.push_back(
            {std::to_string((double)item["p"]), crab::find_asset(symbol_id)});
    }
    return result;
}

[[nodiscard]] auto https_json_parser() -> simdjson::dom::parser&
{
    static auto parser = simdjson::dom::parser{};
    return parser;
}

[[nodiscard]] auto ws_json_parser() -> simdjson::dom::parser&
{
    static auto parser = simdjson::dom::parser{};
    return parser;
}

}  // namespace

namespace crab {

auto Finnhub::stats(Asset const& asset) -> Stats
{
    auto const asset_str = find_symbol_id(asset);
    if (!https_socket_.is_connected())
        this->make_https_connection();
    auto const message = https_socket_.get(
        build_rest_query("quote?symbol=" + asset_str, this->get_key()));
    check_response(message, "Finnhub - Failed to get stats");

    auto const element = https_json_parser().parse(message.body);
    return {(double)element["c"], (double)element["o"]};
}

auto Finnhub::search(std::string const& query) -> std::vector<Search_result>
{
    if (!https_socket_.is_connected())
        this->make_https_connection();
    auto const message = https_socket_.get(
        build_rest_query("search?q=" + query, this->get_key()));
    check_response(message, "Finnhub - Failed to search for: " + query);

    auto result      = std::vector<Search_result>{};
    auto const array = https_json_parser().parse(message.body)["result"];
    for (auto const& x : array) {
        auto const description = (std::string)x["description"];
        auto const symbol_id   = (std::string)x["symbol"];
        auto const type        = (std::string)x["type"];
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

void Finnhub::subscribe(Asset const& asset)
{
    auto const json = std::string{"{\"type\":\"subscribe\",\"symbol\":\""} +
                      find_symbol_id(asset) + "\"}";
    if (!ws_.is_connected())
        this->ws_connect();
    ws_.write(json);
    ++subscription_count_;
}

void Finnhub::unsubscribe(Asset const& asset)
{
    auto const json = std::string{"{\"type\":\"unsubscribe\",\"symbol\":\""} +
                      find_symbol_id(asset) + "\"}";
    if (!ws_.is_connected())
        this->ws_connect();
    ws_.write(json);
    --subscription_count_;
}

auto Finnhub::stream_read() -> std::vector<Price>
{
    auto prices = parse_prices(ws_json_parser().parse(ws_.read()));
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

}  // namespace crab
