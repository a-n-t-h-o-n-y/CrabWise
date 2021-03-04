#include "coinbase.hpp"

#include <algorithm>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <ntwk/check_response.hpp>
#include <ntwk/url_encode.hpp>

#include "error.hpp"

namespace {
using ptree = boost::property_tree::ptree;

/// Build ptree array with this.
void push_back(ptree& array, std::string const& element)
{
    auto x = ptree{};
    x.put("", element);
    array.push_back(std::make_pair("", std::move(x)));
}

auto extract_error(ptree const& tree) -> std::string
{
    return tree.get<std::string>("message") + ": " +
           tree.get<std::string>("reason");
}

auto extract_price(ptree const& tree) -> std::string
{
    return tree.get<std::string>("price");
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

auto extract_base_currency(ptree const& tree) -> std::string
{
    auto const pair_str = tree.get<std::string>("product_id");
    return parse_base_currency(pair_str);
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

auto extract_quote_currency(ptree const& tree) -> std::string
{
    auto const pair_str = tree.get<std::string>("product_id");
    return parse_quote_currency(pair_str);
}

auto parse_trade(ptree const& tree) -> crab::Price
{
    try {
        return {extract_price(tree),
                {"COINBASE",
                 {extract_base_currency(tree), extract_quote_currency(tree)}}};
    }
    catch (std::exception const& e) {
        throw crab::Crab_error{"coinbase.cpp anon::parse_trade(): " +
                               std::string{e.what()}};
    }
}

auto extract_event(ptree const& tree) -> std::string
{
    return tree.get<std::string>("type");
}

// JSON message to Last_price
auto parse(std::string const& json) -> std::optional<crab::Price>
{
    auto tree = ptree{};
    {
        auto ss = std::stringstream{json};
        read_json(ss, tree);
    }

    auto const event = extract_event(tree);
    if (event == "ticker") {
        try {
            return parse_trade(tree);
        }
        catch (crab::Crab_error const&) {
            return std::nullopt;
        }
    }
    else if (event == "subscriptions")
        return std::nullopt;
    else if (event == "error") {
        throw crab::Crab_error{"coinbase.cpp anon::parse(): " +
                               std::string{extract_error(tree)}};
    }
    else
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
    using ptree = boost::property_tree::ptree;

    auto tree = ptree{};
    tree.add("type", "subscribe");

    auto pairs = ptree{};
    push_back(pairs, to_id(asset.currency));
    tree.push_back(std::make_pair("product_ids", pairs));

    auto channels = ptree{};
    push_back(channels, "ticker");

    tree.push_back(std::make_pair("channels", channels));

    auto ss = std::ostringstream{};
    write_json(ss, tree);
    if (!ws_.is_connected())
        ws_.connect("ws-feed.pro.coinbase.com");
    ws_.write(ss.str());
    ++subscription_count_;
}

void Coinbase::unsubscribe(Asset const& asset)
{
    using ptree = boost::property_tree::ptree;

    auto tree = ptree{};
    tree.add("type", "unsubscribe");

    auto pairs = ptree{};
    push_back(pairs, to_id(asset.currency));
    tree.push_back(std::make_pair("product_ids", pairs));

    auto channels = ptree{};
    push_back(channels, "ticker");

    tree.push_back(std::make_pair("channels", channels));

    auto ss = std::ostringstream{};
    write_json(ss, tree);
    if (!ws_.is_connected())
        ws_.connect("ws-feed.pro.coinbase.com");
    ws_.write(ss.str());
    --subscription_count_;
}

auto Coinbase::stream_read() -> Price
{
    if (!ws_.is_connected())
        ws_.connect("ws-feed.pro.coinbase.com");

    auto price = parse(ws_.read());
    while (!price) {
        price = parse(ws_.read());
    }
    return *price;
}

}  // namespace crab
