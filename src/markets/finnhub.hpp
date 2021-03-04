#ifndef CRAB_MARKETS_FINNHUB_HPP
#define CRAB_MARKETS_FINNHUB_HPP
#include <fstream>
#include <string>
#include <vector>

#include <ntwk/https_socket.hpp>
#include <ntwk/websocket.hpp>

#include <simdjson.h>

#include "../asset.hpp"
#include "../price.hpp"
#include "../search_result.hpp"
#include "../stats.hpp"
#include "error.hpp"
#include "symbol_id_cache.hpp"

namespace crab {

class Finnhub {
   public:
    /// Connect internal https socket.
    void make_https_connection() { https_socket_.connect("finnhub.io"); }

    /// Disconnect internal https socket.
    void disconnect_https() { https_socket_.disconnect(); }

    /// Disconnect internal Websocket.
    void disconnect_websocket() { ws_.disconnect(); }

   public:
    /// HTTPS Request for Current and Opening price of Stock or Crypto.
    [[nodiscard]] auto stats(Asset const& asset) -> Stats;

    /// HTTPS Request for Assets, using \p query.
    [[nodiscard]] auto search(std::string const& query)
        -> std::vector<Search_result>;

   public:
    /// Subscribe to websocket update for \p asset.
    void subscribe(Asset const& asset);

    /// Subscribe to websocket update for \p asset.
    void unsubscribe(Asset const& asset);

    /// Read a single response from the websocket, parsed into multiple Prices
    auto stream_read() -> std::vector<Price>;

    /// Return number of subscriptions on the websocket.
    [[nodiscard]] auto subscription_count() const
    {
        return subscription_count_;
    }

   private:
    ntwk::Websocket ws_;
    mutable ntwk::HTTPS_socket https_socket_;
    std::string key_param_;
    int subscription_count_ = 0;

   private:
    [[nodiscard]] static auto parse_key(std::string const& filename)
        -> std::string
    {
        auto file = std::ifstream{filename};
        auto key  = std::string{};
        file >> key;
        return key;
    }

    auto get_key() -> std::string const&
    {
        if (key_param_.empty())
            key_param_ = "token=" + parse_key("finnhub.key");
        return key_param_;
    }

    void ws_connect() { ws_.connect("ws.finnhub.io", "/?" + this->get_key()); }
};

}  // namespace crab
#endif  // CRAB_MARKETS_FINNHUB_HPP
