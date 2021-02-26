#ifndef MARKET_SERVER_MARKET_POLONIEX_HPP
#define MARKET_SERVER_MARKET_POLONIEX_HPP
#include <string>
#include <string_view>

#include <boost/property_tree/ptree.hpp>

#include "../network/websocket.hpp"
#include "detail/signals.hpp"

namespace market_server::market {

class Poloniex {
   public:
    detail::Last_signal_t last_price;
    detail::Open_signal_t opening_price;

    /// Whether or not the market will receive live data from a socket.
    static auto constexpr is_active = true;
    static auto constexpr name      = std::string_view{"Poloniex"};

   public:
    /// Make connection and subscribe to channels.
    void initialize();

    /// Read a single response from the bitstamp server.
    auto read() -> bool { return ws_.read(); }

    /// Perform reconnection to endpoint.
    void reconnect();

   private:
    inline static auto const ws_endpoint_    = std::string{"api2.poloniex.com"};
    inline static auto const https_endpoint_ = std::string{"www.poloniex.com"};

    network::Websocket ws_;

   private:
    /// Called on each received message from ws_.
    void parse_and_emit(std::string const& message);

    /// Parses all prices from json and sends each via the response signal.
    void initial_price_parse_and_emit(boost::property_tree::ptree const& root);

    /// Emit opening_price signal for each price.
    void opening_price_parse_and_emit(boost::property_tree::ptree const& root);

    /// Retreive current prices for all symbols via a REST API.
    /** Emits the response signal for each, which will populate the cache but
     *  not actually publish the price on the zmq socket. status return */
    void cache_initial_prices();
};

}  // namespace market_server::market
#endif  // MARKET_SERVER_MARKET_POLONIEX_HPP
