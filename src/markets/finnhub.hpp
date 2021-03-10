#ifndef CRAB_MARKETS_FINNHUB_HPP
#define CRAB_MARKETS_FINNHUB_HPP
#include <cassert>
#include <exception>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <ntwk/https_socket.hpp>
#include <ntwk/websocket.hpp>

#include <simdjson.h>

#include "../asset.hpp"
#include "../filenames.hpp"
#include "../log.hpp"
#include "../price.hpp"
#include "../search_result.hpp"
#include "../stats.hpp"
#include "../symbol_id_json.hpp"
#include "error.hpp"
#include "symbol_id_cache.hpp"

namespace crab {

class Finnhub {
   public:
    /// Connect internal https socket.
    void make_https_connection()
    {
        log_status("HTTPS connect: finnhub.io");
        try {
            https_socket_.connect("finnhub.io");
        }
        catch (std::exception const& e) {
            log_error("Finnhub failed to connect over HTTPS: " +
                      std::string{e.what()});
        }
    }

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
    int subscription_count_   = 0;
    Symbol_ID_cache id_cache_ = read_ids_json(symbol_ids_json_filepath());

   private:
    [[nodiscard]] static auto parse_key(std::filesystem::path const& filepath)
        -> std::string
    {
        auto file = std::ifstream{filepath};
        auto key  = std::string{};
        file >> key;
        if (key.empty())
            throw std::runtime_error{"Empty Finnhub API Key in finnhub.key"};
        return key;
    }

    auto get_key_param() -> std::string const&
    {
        if (key_param_.empty()) {
            try {
                key_param_ = "token=" + parse_key(finnhub_key_filepath());
            }
            catch (std::exception const& e) {
                log_error(e.what());
            }
        }
        return key_param_;
    }

    void ws_connect()
    {
        log_status("Websocket connect: ws.finnhub.io");
        try {
            ws_.connect("ws.finnhub.io", "/?" + this->get_key_param());
        }
        catch (std::exception const& e) {
            log_error("Finnhub Websocket failed to connect: " +
                      std::string{e.what()});
        }
    }
};

}  // namespace crab
#endif  // CRAB_MARKETS_FINNHUB_HPP
