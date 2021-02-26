#ifndef MARKET_SERVER_MARKET_BITSTAMP_HPP
#define MARKET_SERVER_MARKET_BITSTAMP_HPP
#include <string>
#include <string_view>

#include "../network/websocket.hpp"
#include "detail/signals.hpp"

namespace market_server::market {

class Bitstamp {
   public:
    detail::Last_signal_t last_price;
    detail::Open_signal_t opening_price;

    /// Whether or not the market will receive live data from a socket.
    static auto constexpr is_active = true;
    static auto constexpr name      = std::string_view{"Bitstamp"};

   public:
    /// Make connection and subscribe to channels.
    void initialize();

    /// Read a single response from the bitstamp server.
    auto read() -> bool { return ws_.read(); }

    /// Perform reconnection to endpoint.
    void reconnect();

   private:
    network::Websocket ws_;

    /// Called on each received message from ws_.
    void parse_and_emit(std::string const& json);

    /// Takes lower case \p base and \p quote.
    void subscribe(std::string const& base, std::string const& quote);

    /// Request prices for all available currency pairs.
    void subscribe_to_all();

    /// Retreive current prices for all symbols via a REST API.
    /** Emits the response signal for each, which will populate the cache but
     *  not actually publish the price on the zmq socket. status return */
    auto cache_initial_prices() -> bool;

    /// Json to message::Last_price
    auto parse(std::string const& json)
        -> std::optional<sckt::message::Last_price>;
};

}  // namespace market_server::market
#endif  // MARKET_SERVER_MARKET_BITSTAMP_HPP
