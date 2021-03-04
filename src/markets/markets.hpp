#ifndef CRAB_MARKETS_MARKETS_HPP
#define CRAB_MARKETS_MARKETS_HPP
#include <mutex>
#include <string>
#include <vector>

#include <signals_light/signal.hpp>
#include <termox/system/event_loop.hpp>

#include "../asset.hpp"
#include "../stats.hpp"
#include "coinbase.hpp"
#include "finnhub.hpp"

namespace crab::detail {
/// List of assets to subscribe to with mutex lock.
class Locking_asset_list {
   public:
    auto lock() { return std::lock_guard{mtx_}; }

   public:
    void push_back(Asset asset) { assets_.push_back(std::move(asset)); }

    void clear() { assets_.clear(); }

   public:
    auto begin() const { return std::begin(assets_); }

    auto end() const { return std::end(assets_); }

    auto size() const { return assets_.size(); }

   private:
    std::vector<Asset> assets_;
    std::mutex mtx_;
};

}  // namespace crab::detail

namespace crab {

/// Wrapper around all concrete markets, makes decisions on which market to use.
class Markets {
   public:
    sl::Signal<void(Price const&)> price_update;

   private:
    std::mutex price_update_mtx_;  // locked when emitting

   public:
    void shutdown()
    {
        finnhub_.disconnect_https();
        finnhub_loop_.exit(0);
        finnhub_loop_.wait();
        finnhub_.disconnect_websocket();

        coinbase_loop_.exit(0);
        coinbase_loop_.wait();
        coinbase_.disconnect_websocket();
    }

    [[nodiscard]] auto stats(Asset const& asset) -> Stats
    {
        return finnhub_.stats(asset);
    }

    [[nodiscard]] auto search(std::string const& query)
        -> std::vector<Search_result>
    {
        return finnhub_.search(query);
    }

    void launch_streams()
    {
        this->launch_finnhub();
        this->launch_coinbase();
    }

    void subscribe(Asset const& asset)
    {
        if (asset.exchange == "COINBASE") {
            auto const lock = coinbase_to_subscribe_to_.lock();
            coinbase_to_subscribe_to_.push_back(asset);
        }
        else {
            auto const lock = finnhub_to_subscribe_to_.lock();
            finnhub_to_subscribe_to_.push_back(asset);
        }
    }

    void unsubscribe(Asset const& asset)
    {
        if (asset.exchange == "COINBASE") {
            auto const lock = coinbase_to_unsubscribe_to_.lock();
            coinbase_to_unsubscribe_to_.push_back(asset);
        }
        else {
            auto const lock = finnhub_to_unsubscribe_to_.lock();
            finnhub_to_unsubscribe_to_.push_back(asset);
        }
    }

   private:
    Finnhub finnhub_;
    Coinbase coinbase_;

    ox::Event_loop finnhub_loop_;
    ox::Event_loop coinbase_loop_;

    detail::Locking_asset_list coinbase_to_subscribe_to_;
    detail::Locking_asset_list coinbase_to_unsubscribe_to_;

    detail::Locking_asset_list finnhub_to_subscribe_to_;
    detail::Locking_asset_list finnhub_to_unsubscribe_to_;

   private:
    template <typename Market_t, typename Locking_queue_t>
    auto generate_loop_fn(Market_t& market,
                          Locking_queue_t& to_sub,
                          Locking_queue_t& to_unsub)
    {
        return [this, &market, &to_sub, &to_unsub](ox::Event_queue& q) {
            {  // Subscribe
                auto const lock = to_sub.lock();
                for (auto const& asset : to_sub)
                    market.subscribe(asset);
                to_sub.clear();
            }
            {  // Unsubscribe
                auto const lock = to_unsub.lock();
                for (auto const& asset : to_unsub)
                    market.unsubscribe(asset);
                to_unsub.clear();
            }
            if (market.subscription_count() != 0) {
                auto const prices = market.stream_read();
                q.append(ox::Custom_event{[this, prices] {
                    auto const lock = std::lock_guard{price_update_mtx_};
                    if constexpr (std::is_same_v<decltype(prices), const Price>)
                        this->price_update(prices);
                    else {
                        for (auto const& p : prices)
                            this->price_update(p);
                    }
                }});
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
            }
        };
    }

    void launch_finnhub()
    {
        if (finnhub_loop_.is_running())
            return;
        finnhub_loop_.run_async(this->generate_loop_fn(
            finnhub_, finnhub_to_subscribe_to_, finnhub_to_unsubscribe_to_));
    }

    void launch_coinbase()
    {
        if (coinbase_loop_.is_running())
            return;
        coinbase_loop_.run_async(this->generate_loop_fn(
            coinbase_, coinbase_to_subscribe_to_, coinbase_to_unsubscribe_to_));
    }
};

}  // namespace crab
#endif  // CRAB_MARKETS_MARKETS_HPP
