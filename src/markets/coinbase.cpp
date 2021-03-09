#include "coinbase.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <simdjson.h>

#include "../log.hpp"
#include "error.hpp"

namespace {

using JSON_element_t = simdjson::simdjson_result<simdjson::dom::element>;

// Parser specifically for coinbase thread. Make sure you aren't using in
// multiple threads.
[[nodiscard]] auto json_parser() -> simdjson::dom::parser&
{
    static auto parser = simdjson::dom::parser{};
    return parser;
}

auto extract_error(JSON_element_t const& e) -> std::string
{
    return (std::string)e["message"] + ": " + (std::string)e["reason"];
}

auto extract_price(JSON_element_t const& e) -> std::string
{
    return (std::string)e["price"];
}

// Split on '-': "XTZ-BTC" extracts "XTZ"
auto parse_base_currency(std::string const& pair_str) -> std::string
{
    auto const begin = std::cbegin(pair_str);
    auto const end   = std::cend(pair_str);
    auto const iter  = std::find(begin, end, '-');
    if (iter == end) {
        throw crab::Crab_error{"Coinbase: Can't Parse Base Currency from: " +
                               pair_str};
    }
    return {begin, iter};
}

auto extract_base_currency(JSON_element_t const& e) -> std::string
{
    return parse_base_currency((std::string)e["product_id"]);
}

// Split on '-'
auto parse_quote_currency(std::string const& pair_str) -> std::string
{
    auto const end = std::end(pair_str);
    auto iter      = std::find(std::begin(pair_str), end, '-');
    if (iter == end)
        throw crab::Crab_error{"Coinbase: Can't parse Currency"};
    std::advance(iter, 1);
    return std::string{iter, end};
}

auto extract_quote_currency(JSON_element_t const& e) -> std::string
{
    return parse_quote_currency((std::string)e["product_id"]);
}

auto parse_trade(JSON_element_t const& e) -> std::optional<crab::Price>
{
    try {
        return crab::Price{
            extract_price(e),
            {"COINBASE",
             {extract_base_currency(e), extract_quote_currency(e)}}};
    }
    catch (std::exception const& e) {
        crab::log_error("Coinbase could not parse trade json element.");
        return std::nullopt;
    }
}

// JSON message to Last_price
auto parse(std::string const& json) -> std::optional<crab::Price>
{
    auto const element = json_parser().parse(json);
    auto const event   = (std::string)element["type"];
    if (event == "ticker")
        return parse_trade(element);
    if (event == "error") {
        throw crab::Crab_error{"coinbase.cpp anon::parse(): " +
                               extract_error(element)};
    }
    return std::nullopt;
}

/// Currency_pair to coinbase formatted id: [base]-[quote]
[[nodiscard]] auto to_id(crab::Currency_pair currency) -> std::string
{
    return currency.base + '-' + currency.quote;
}

}  // namespace

namespace crab {

void Coinbase::subscribe(Asset const& asset)
{
    if (!ws_.is_connected())
        this->ws_connect();
    auto const json =
        std::string{"{\"type\":\"subscribe\",\"product_ids\":[\""} +
        to_id(asset.currency) + "\"],\"channels\":[\"ticker\"]}";
    log_status("Coinbase Websocket Subscribing: " + asset.exchange + ' ' +
               asset.currency.base + ' ' + asset.currency.quote);
    try {
        ws_.write(json);
        ++subscription_count_;
    }
    catch (std::exception const& e) {
        log_error("Coinbase failed to subscribe to: " + asset.currency.quote +
                  ' ' + asset.currency.quote + ' ' + e.what());
    }
}

void Coinbase::unsubscribe(Asset const& asset)
{
    if (!ws_.is_connected())
        this->ws_connect();
    auto const json =
        std::string{"{\"type\":\"unsubscribe\",\"product_ids\":[\""} +
        to_id(asset.currency) + "\"],\"channels\":[\"ticker\"]}";
    log_status("Coinbase Websocket Unsubscribing: " + asset.exchange + ' ' +
               asset.currency.base + ' ' + asset.currency.quote);
    try {
        ws_.write(json);
        --subscription_count_;
    }
    catch (std::exception const& e) {
        log_error("Coinbase failed to unsubscribe to: " + asset.currency.quote +
                  ' ' + asset.currency.quote + ' ' + e.what());
    }
}

auto Coinbase::stream_read() -> Price
{
    if (!ws_.is_connected())
        this->ws_connect();

    auto price = std::optional<Price>{std::nullopt};
    while (!price) {
        try {
            price = parse(ws_.read());
        }
        catch (std::exception const& e) {
            log_error("Coinbase Failed to read from Websocket: " +
                      std::string{e.what()});
            return {};
        }
    }
    return *price;
}

}  // namespace crab
