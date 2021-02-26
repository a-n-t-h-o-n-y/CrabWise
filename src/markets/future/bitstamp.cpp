#include "bitstamp.hpp"

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <socket/make_symbol.hpp>
#include <socket/message.hpp>

#include <utility/log.hpp>
#include <utility/to_upper.hpp>

#include "../network/https_socket.hpp"

namespace {
using ptree = boost::property_tree::ptree;

auto extract_event(ptree const& tree) -> std::string
{
    return tree.get<std::string>("event");
}

auto extract_price(ptree const& tree) -> sckt::Price_t
{
    return tree.get<sckt::Price_t>("data.price");
}

auto extract_base_currency(ptree const& tree) -> std::string
{
    // Channel Example: "live_trades_btcusd"
    auto const channel = tree.get<std::string>("channel");
    if (channel.size() != 18)
        throw std::runtime_error{"Bitstamp: Can't parse base currency."};
    return utility::to_upper(channel.substr(12, 3));
}

auto extract_quote_currency(ptree const& tree) -> sckt::Currency
{
    auto const channel = tree.get<std::string>("channel");
    // Channel Example: "live_trades_btcusd"
    if (channel.size() != 18)
        throw std::runtime_error{"Bitstamp: Can't parse quote currency"};
    return sckt::StringToCurrency(utility::to_upper(channel.substr(15, 3)));
}

auto parse_trade(ptree const& tree) -> std::optional<sckt::message::Last_price>
{
    try {
        auto const exchange = sckt::MarketDataType::BITSTAMP;
        auto const price    = extract_price(tree);
        auto const base     = extract_base_currency(tree);
        auto const quote    = extract_quote_currency(tree);
        return sckt::message::Last_price{
            sckt::make_symbol(exchange, base, quote), price};
    }
    catch (...) {
        return std::nullopt;
    }
}

struct Initial_data {
    sckt::Symbol symbol;
    sckt::Price_t last_price;
    sckt::Price_t opening_price;
};

// base and quote should be lower case
auto get_currency_and_opening_price(
    std::string const& base,
    std::string const& quote,
    market_server::network::HTTPS_socket& socket) -> Initial_data
{
    auto const request = "/api/v2/ticker/" + base + quote + "/";
    auto response      = market_server::network::HTTPS_socket::Response{0, ""};
    while (response.code != 200) {
        if (response.code == 400) {
            utility::Log::market{
                "bitstamp.cpp anon::get_currency_and_opening_price()"}
                << "Bad Response, Trying again...";
        }
        else if (response.code != 0) {
            utility::Log::market{
                "bitstamp.cpp anon::get_currency_and_opening_price()"}
                << "Initialization HTTP Error" << response.code
                << response.body;
        }
        response = socket.get(request);
    }

    auto tree = boost::property_tree::ptree{};
    auto ss   = std::stringstream{response.body};
    read_json(ss, tree);
    auto const last = tree.get<sckt::Price_t>("last");
    auto const open = tree.get<sckt::Price_t>("open");
    return {sckt::make_symbol("BITSTAMP", utility::to_upper(base),
                              utility::to_upper(quote)),
            last, open};
}

}  // namespace
namespace market_server::market {

void Bitstamp::initialize()
{
    if (!this->cache_initial_prices())
        throw std::runtime_error{"Bitstamp::initialize: Can't Cache"};

    ws_.response.connect([this](auto const& x) { this->parse_and_emit(x); });
    if (!ws_.connect("ws.bitstamp.net")) {
        utility::Log::market{"Bitstamp::initialize()"}
            << "Can't Connect to Websocket";
        return;
    }
    this->subscribe_to_all();
}

void Bitstamp::reconnect()
{
    auto log = utility::Log::market{"Bitstamp::reconnect()"};
    ws_.disconnect();
    if (!ws_.connect("ws.bitstamp.net")) {
        log << "Failed to Reconnect";
        throw std::runtime_error{"Bitstamp::Reconnect Failed"};
    }
    this->subscribe_to_all();
    log << "Reconnect Succeeded";
}

void Bitstamp::subscribe_to_all()
{
    this->subscribe("btc", "usd");
    this->subscribe("btc", "eur");

    this->subscribe("eur", "usd");

    this->subscribe("xrp", "usd");
    this->subscribe("xrp", "eur");
    this->subscribe("xrp", "btc");

    this->subscribe("ltc", "usd");
    this->subscribe("ltc", "eur");
    this->subscribe("ltc", "btc");

    this->subscribe("eth", "usd");
    this->subscribe("eth", "eur");
    this->subscribe("eth", "btc");

    this->subscribe("bch", "usd");
    this->subscribe("bch", "eur");
    this->subscribe("bch", "btc");
}

void Bitstamp::parse_and_emit(std::string const& json)
{
    auto const price = this->parse(json);
    if (price)
        this->last_price(*price);
}

void Bitstamp::subscribe(std::string const& base, std::string const& quote)
{
    auto tree = ptree{};
    tree.add("event", "bts:subscribe");

    auto data_tree = ptree{};
    data_tree.add("channel", "live_trades_" + base + quote);
    tree.add_child("data", data_tree);

    auto ss = std::ostringstream{};
    write_json(ss, tree);
    ws_.write(ss.str());
    ws_.read();
}

auto Bitstamp::cache_initial_prices() -> bool
{
    auto socket        = network::HTTPS_socket{};
    auto const success = socket.connect("www.bitstamp.net");
    if (!success) {
        utility::Log::market{"Bitstamp::cache_initial_prices()"}
            << "Can't Connect to Bitstamp HTTPS socket"
            << "www.bitstamp.net";
        return false;
    }
    auto const init = [&](auto const& base, auto const& quote) {
        auto const data = get_currency_and_opening_price(base, quote, socket);
        this->last_price({data.symbol, data.last_price});
        this->opening_price({data.symbol, data.opening_price});
    };

    init("btc", "usd");
    init("btc", "eur");

    init("eur", "usd");

    init("xrp", "usd");
    init("xrp", "eur");
    init("xrp", "btc");

    init("ltc", "usd");
    init("ltc", "eur");
    init("ltc", "btc");

    init("eth", "usd");
    init("eth", "eur");
    init("eth", "btc");

    init("bch", "usd");
    init("bch", "eur");
    init("bch", "btc");

    socket.disconnect();
    return true;
}

auto Bitstamp::parse(std::string const& json)
    -> std::optional<sckt::message::Last_price>
{
    auto tree = ptree{};
    {
        auto ss = std::stringstream{json};
        read_json(ss, tree);
    }
    auto const event = extract_event(tree);
    if (event == "trade")
        return parse_trade(tree);
    else if (event == "bts:subscription_succeeded")
        return std::nullopt;  // Subscriptions don't seem to be able to fail.
    else if (event == "bts:request_reconnect") {
        utility::Log::market{"bitstamp.cpp anon::parse()"}
            << "bts::request_reconnect Message Received";
        this->reconnect();
        return std::nullopt;
    }
    else {
        utility::Log::market{"bitstamp.cpp anon::parse()"}
            << "Unknown Message Received" << event;
        return std::nullopt;
    }
}

}  // namespace market_server::market
