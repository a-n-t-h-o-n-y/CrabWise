#ifndef CRAB_MARKETS_COINBASE_HPP
#define CRAB_MARKETS_COINBASE_HPP
#include <exception>
#include <string>

#include <ntwk/websocket.hpp>

#include "../asset.hpp"
#include "../log.hpp"
#include "../price.hpp"

namespace crab {

/// Provides access to Coinbase websocket API for live prices.
class Coinbase {
   public:
    void disconnect_websocket() { ws_.disconnect(); }

    /// Start listening for live prices for \p asset.
    /** Connects websocket if not connected yet. */
    void subscribe(Asset const& asset);

    /// Stop listening for live prices for \p asset.
    /** Connects websocket if not connected yet. */
    void unsubscribe(Asset const& asset);

    [[nodiscard]] auto subscription_count() const -> int
    {
        return subscription_count_;
    }

    /// Read a single response from the coinbase server.
    [[nodiscard]] auto stream_read() -> Price;

   private:
    ntwk::Websocket ws_;
    int subscription_count_ = 0;

   private:
    void ws_connect()
    {
        log_status("Websocket connect: ws-feed.pro.coinbase.com");
        try {
            ws_.connect("ws-feed.pro.coinbase.com");
        }
        catch (std::exception const& e) {
            log_error("Coinbase Websocket failed to connect: " +
                      std::string{e.what()});
        }
    }
};

}  // namespace crab
#endif  // CRAB_MARKETS_COINBASE_HPP
