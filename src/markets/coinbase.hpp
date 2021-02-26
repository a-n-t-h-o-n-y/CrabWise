#ifndef CRAB_MARKETS_COINBASE_HPP
#define CRAB_MARKETS_COINBASE_HPP
#include <string>
#include <string_view>
#include <vector>

#include "../candle.hpp"
#include "../currency_pair.hpp"
#include "../period.hpp"
#include "../price.hpp"
#include "network/https_socket.hpp"
#include "network/websocket.hpp"

namespace crab {

class Coinbase {
   public:
    static auto constexpr name = std::string_view{"Coinbase"};

   public:
    /// Connect internal https socket.
    void make_https_connection()
    {
        https_socket_.connect("api.pro.coinbase.com");
    }

    /// Disconnect internal https socket.
    void disconnect_https() { https_socket_.disconnect(); }

   public:
    /// Return list of supported currency pairs.
    /** Makes html request if not initialized yet. */
    [[nodiscard]] auto currency_pairs() -> std::vector<Currency_pair> const&
    {
        if (currency_pairs_.empty())
            currency_pairs_ = this->request_currency_pairs();
        return currency_pairs_;
    }

    /// Return the price of the given Currency_pair at the given iso8601 string.
    [[nodiscard]] auto price_at(Currency_pair currency, std::string const& at)
        -> Price;

    /// Return the price of the given Currency_pair at the time point.
    [[nodiscard]] auto price_at(Currency_pair currency, Candle::Time_point_t at)
        -> Price;

    /// Return the current price.
    [[nodiscard]] auto current_price(Currency_pair currency) -> Price;

    /// Return the UTC Midnight opening price of the given currency pair.
    /** Caches results, retrieves new results if cache is from yesterday.
     *  4pm PST is UTC midnight. */
    [[nodiscard]] auto opening_price(Currency_pair currency) -> Price;

    // TODO - don't need this yet.
    /// Return historical data for a given time period and Currency_pair. Values
    /// are cached. Granularity is implementation defined.
    [[nodiscard]] auto candles(Currency_pair, Period) -> std::vector<Candle>;

    /// Starts connection to websocket if not started yet. Registers to start
    /// receiving live price data from read() for Currency_pair.
    void subscribe(Currency_pair currency);

    /// Read a single response from the coinbase server.
    auto read() -> Price;

   private:
    Websocket ws_;
    std::vector<Currency_pair> currency_pairs_;
    HTTPS_socket https_socket_;

   private:
    /// use https socket to get currency_pairs_.
    [[nodiscard]] auto request_currency_pairs() -> std::vector<Currency_pair>;
};

}  // namespace crab
#endif  // CRAB_MARKETS_COINBASE_HPP
