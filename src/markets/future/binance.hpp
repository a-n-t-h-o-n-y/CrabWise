#ifndef MARKET_SERVER_MARKET_BINANCE_HPP
#define MARKET_SERVER_MARKET_BINANCE_HPP
#include <string>
#include <string_view>

#include <socket/message.hpp>

#include "../network/websocket.hpp"
#include "detail/signals.hpp"

namespace market_server::network {
class HTTPS_socket;
}

namespace market_server::market {

// TODO websocket will disconnect after 24 hours.
class Binance {
   public:
    detail::Last_signal_t last_price;
    detail::Open_signal_t opening_price;

    /// Whether or not the market will receive live data from a socket.
    static auto constexpr is_active = true;
    static auto constexpr name      = std::string_view{"Binance"};

   public:
    /// Make connection and subscribe to channels.
    void initialize();

    /// Read a single response from the binance server.
    auto read() -> bool { return ws_.read(); }

    /// Perform reconnection to endpoint.
    void reconnect();

   private:
    network::Websocket ws_;

   private:
    /// Called on each received message from ws_.
    void parse_and_emit(std::string const& json);

    /// Gets supported symbols and initialized the cache.
    void get_initial_info();

    /// Populate currency_pairs_ with all supported currency pairs.
    void build_currency_pair_map(network::HTTPS_socket& sock);

    /// Emit current price of each supported symbol.
    void cache_initial_prices(network::HTTPS_socket& sock);

    /// Emit opening prices of each symbol
    void cache_opening_prices(network::HTTPS_socket& sock);
};

}  // namespace market_server::market
#endif  // MARKET_SERVER_MARKET_BINANCE_HPP
