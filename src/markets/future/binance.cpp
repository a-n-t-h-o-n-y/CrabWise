#include "binance.hpp"

#include <cctype>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <socket/make_symbol.hpp>
#include <socket/message.hpp>

#include <utility/log.hpp>
#include <utility/to_lower.hpp>

#include "../network/https_socket.hpp"

namespace {
using ptree = boost::property_tree::ptree;

struct Currency_pair {
    std::string base;
    sckt::Currency quote;
};

std::map<std::string, Currency_pair> currency_pairs;

auto build_subscriptions_endpoint() -> std::string
{
    auto result = std::string{"/stream?streams="};
    for (auto const& pair : currency_pairs)
        result += utility::to_lower(pair.first) + "@trade" + '/';
    result.pop_back();
    return result;
}

auto extract_price(ptree const& tree) -> sckt::Price_t
{
    return tree.get<sckt::Price_t>("p");
}

auto extract_pair(ptree const& tree) -> Currency_pair
{
    auto const sym = tree.get<std::string>("s");
    return currency_pairs.at(sym);
}

using Last_price = sckt::message::Last_price;

auto parse_trade(ptree const& tree) -> std::optional<Last_price>
{
    try {
        auto const exchange   = sckt::MarketDataType::BINANCE;
        auto const price      = extract_price(tree);
        auto const base_quote = extract_pair(tree);
        return Last_price{
            sckt::make_symbol(exchange, base_quote.base, base_quote.quote),
            price};
    }
    catch (std::exception const& e) {
        utility::Log::market{"binance.cpp anon::parse_trade()"} << e.what();
        return std::nullopt;
    }
}

auto extract_type(ptree const& tree) -> std::string
{
    auto const type = tree.get<std::string>("stream");
    auto const pos  = type.find('@');
    if (pos == std::string::npos)
        return type;
    return type.substr(pos);
}

// JSON message to Last_price
auto parse(std::string const& json) -> std::optional<Last_price>
{
    auto tree = ptree{};
    {
        auto ss = std::stringstream{json};
        read_json(ss, tree);
    }

    auto const type = extract_type(tree);
    if (type == "@trade")
        return parse_trade(tree.get_child("data"));
    else
        return std::nullopt;
}

}  // namespace

namespace market_server::market {

void Binance::initialize()
{
    this->get_initial_info();
    ws_.response.connect([this](auto const& x) { this->parse_and_emit(x); });
    // TODO move this to larger scope, market server.
    // provide string names for markets then. constexpr name() string_view
    if (!ws_.connect("stream.binance.com", build_subscriptions_endpoint(),
                     "9443")) {
        utility::Log::market{"Binance::initialize()"}
            << "Can't connect to Websocket: stream.binance.com";
        return;
    }
}

void Binance::reconnect()
{
    auto log = utility::Log::market{"Binance::reconnect()"};
    ws_.disconnect();
    if (!ws_.connect("stream.binance.com", build_subscriptions_endpoint(),
                     "9443")) {
        log << "Failed to Reconnect";
        throw std::runtime_error{"Binance::Reconnect Failed"};
    }
    log << "Reconnect Succeeded";
}

void Binance::parse_and_emit(std::string const& json)
{
    auto const price = parse(json);
    if (price)
        this->last_price(*price);
}

void Binance::get_initial_info()
{
    network::HTTPS_socket sock;
    sock.connect("api.binance.com");
    this->build_currency_pair_map(sock);
    this->cache_initial_prices(sock);
    this->cache_opening_prices(sock);
}

void Binance::build_currency_pair_map(network::HTTPS_socket& sock)
{
    currency_pairs.clear();
    auto const message = sock.get("/api/v1/exchangeInfo");
    if (message.code != 200) {
        // TODO This kind of thing should be an exception
        utility::Log::market{"Binance::build_currency_pair_map()"}
            << "Request Failed with code: " + std::to_string(message.code)
            << "endpoint: api.binance.com/api/v1/exchangeInfo";
        return;
    }
    auto tree = ptree{};
    {
        auto ss = std::stringstream{message.body};
        read_json(ss, tree);
    }
    auto const symbols_tree = tree.get_child("symbols");
    for (auto const& symbol : symbols_tree) {
        auto const base  = symbol.second.get<std::string>("baseAsset");
        auto const quote = symbol.second.get<std::string>("quoteAsset");
        try {
            auto const pair =
                Currency_pair{base, sckt::StringToCurrency(quote)};
            auto const text      = symbol.second.get<std::string>("symbol");
            currency_pairs[text] = std::move(pair);
        }
        catch (std::runtime_error const& e) {
            utility::Log::market{"Binance::build_currency_pair_map()"}
                << "Unknown Quote Currency Received" << e.what()
                << "Base: " + base;
        }
    }
}

void Binance::cache_initial_prices(network::HTTPS_socket& sock)
{
    auto const message = sock.get("/api/v3/ticker/price");
    if (message.code != 200) {
        utility::Log::market{"Binance::cache_initial_prices()"}
            << "Request Failed with code: " + std::to_string(message.code)
            << "endpoint: api.binance.com/api/v3/ticker/price";
        return;
    }
    auto tree = ptree{};
    {
        auto ss = std::stringstream{message.body};
        read_json(ss, tree);
    }
    for (auto const& price_tree : tree) {
        auto const& price_data = price_tree.second;
        auto const symbol      = price_data.get<std::string>("symbol");
        try {
            auto const pair  = currency_pairs.at(symbol);
            auto const price = price_data.get<sckt::Price_t>("price");
            this->last_price(
                Last_price{sckt::make_symbol(sckt::MarketDataType::BINANCE,
                                             pair.base, pair.quote),
                           price});
        }
        catch (std::out_of_range const& e) {
            utility::Log::market{"Binance::cache_initial_prices()"}
                << "Can't find symbol in currency_pairs map" << e.what()
                << "Symbol: " + symbol;
        }
    }
}

void Binance::cache_opening_prices(network::HTTPS_socket& sock)
{
    auto const message = sock.get("/api/v1/ticker/24hr");
    if (message.code != 200) {
        utility::Log::market{"Binance::cache_opening_prices()"}
            << "Request Failed with code: " + std::to_string(message.code)
            << "endpoint: api.binance.com/api/v1/ticker/24hr";
        return;
    }
    auto tree = ptree{};
    {
        auto ss = std::stringstream{message.body};
        read_json(ss, tree);
    }
    for (auto const& price_tree : tree) {
        auto const& price_data = price_tree.second;
        auto const symbol      = price_data.get<std::string>("symbol");
        try {
            auto const pair    = currency_pairs.at(symbol);
            auto const price   = price_data.get<sckt::Price_t>("openPrice");
            auto const opening = sckt::message::Opening_price{
                sckt::make_symbol(sckt::MarketDataType::BINANCE, pair.base,
                                  pair.quote),
                price};
            this->opening_price(opening);
        }
        catch (std::out_of_range const& e) {
            utility::Log::market{"Binance::cache_opening_prices()"}
                << "Can't find symbol in currency_pairs map" << e.what()
                << "Symbol: " + symbol;
        }
    }
}

}  // namespace market_server::market
