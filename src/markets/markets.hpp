#ifndef CRAB_MARKETS_MARKETS_HPP
#define CRAB_MARKETS_MARKETS_HPP
#include <mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <signals_light/signal.hpp>
#include <termox/system/event_loop.hpp>

#include "../asset.hpp"
#include "../stats.hpp"
#include "coinbase.hpp"
#include "finnhub.hpp"
#include "termox/system/event.hpp"

namespace crab::detail {

/// List of values with mutex lock.
template <typename T>
class Locking_list {
   public:
    auto lock() { return std::lock_guard{mtx_}; }

   public:
    void push_back(T value) { values_.push_back(std::move(value)); }

    void clear() { values_.clear(); }

   public:
    auto begin() const { return std::cbegin(values_); }

    auto begin() { return std::begin(values_); }

    auto end() const { return std::cend(values_); }

    auto end() { return std::end(values_); }

    auto front() const { return values_.front(); }

    auto front() { return values_.front(); }

    auto back() const { return values_.back(); }

    auto back() { return values_.back(); }

    auto size() const { return values_.size(); }

    auto empty() const { return values_.empty(); }

   private:
    std::vector<T> values_;
    std::mutex mtx_;
};

using Locking_asset_list  = Locking_list<Asset>;
using Locking_string_list = Locking_list<std::string>;

}  // namespace crab::detail

namespace crab {

/// Wrapper around all concrete markets, makes decisions on which market to use.
class Markets {
   public:
    sl::Signal<void(Price const&)> price_update;
    sl::Signal<void(Asset const&, Stats const&)> stats_received;
    sl::Signal<void(std::vector<Search_result> const&)> search_results_received;

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

        stats_loop_.exit(0);
        stats_loop_.wait();

        search_loop_.exit(0);
        search_loop_.wait();
    }

    void request_stats(Asset const& asset)
    {
        auto const lock = stats_requested_.lock();
        if (!stats_loop_.is_running())
            this->launch_stats_loop();
        stats_requested_.push_back(asset);
    }

    void request_search(std::string const& query)
    {
        auto const lock = search_requested_.lock();
        if (!search_loop_.is_running())
            this->launch_search_loop();
        search_requested_.push_back(query);
    }

    void launch_streams()
    {
        this->launch_stats_loop();
        this->launch_search_loop();
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

    detail::Locking_asset_list stats_requested_;
    ox::Event_loop stats_loop_;

    detail::Locking_string_list search_requested_;
    ox::Event_loop search_loop_;

    // search and stats both use the finnhub https socket.
    std::mutex https_socket_mtx_;

   private:
    // TODO should take event loop reference so it can exit itself.
    template <typename Market_t, typename Locking_queue_t>
    auto generate_loop_fn(Market_t& market,
                          Locking_queue_t& to_sub,
                          Locking_queue_t& to_unsub,
                          ox::Event_loop& loop)
    {
        return [this, &market, &to_sub, &to_unsub, &loop](ox::Event_queue& q) {
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
            if (market.subscription_count() == 0)
                std::this_thread::sleep_for(std::chrono::milliseconds{100});
            else {
                try {
                    auto const prices = market.stream_read();
                    q.append(ox::Custom_event{[this, prices] {
                        auto const lock = std::lock_guard{price_update_mtx_};
                        if constexpr (std::is_same_v<decltype(prices),
                                                     const Price>)
                            this->price_update(prices);
                        else {
                            for (auto const& p : prices)
                                this->price_update(p);
                        }
                    }});
                }
                catch (std::exception const&) {
                    loop.exit(1);
                }
            }
        };
    }

    void launch_finnhub()
    {
        if (finnhub_loop_.is_running())
            return;
        finnhub_loop_.run_async(
            this->generate_loop_fn(finnhub_, finnhub_to_subscribe_to_,
                                   finnhub_to_unsubscribe_to_, finnhub_loop_));
    }

    void launch_coinbase()
    {
        if (coinbase_loop_.is_running())
            return;
        coinbase_loop_.run_async(this->generate_loop_fn(
            coinbase_, coinbase_to_subscribe_to_, coinbase_to_unsubscribe_to_,
            coinbase_loop_));
    }

    void launch_stats_loop()
    {
        if (stats_loop_.is_running())
            return;
        stats_loop_.run_async([this](ox::Event_queue& q) {
            {
                // Make sure to never post events until inside Custom_event
                auto stats_requested_copy = std::vector<Asset>{};
                {  // Make a copy, b/c making requests here holds up the lock.
                    auto const lock = stats_requested_.lock();
                    for (Asset& a : stats_requested_)
                        stats_requested_copy.push_back(std::move(a));
                    stats_requested_.clear();
                }
                if (stats_requested_copy.empty())
                    std::this_thread::sleep_for(std::chrono::milliseconds{300});
                else {
                    auto results = std::vector<std::pair<Asset, Stats>>{};
                    for (Asset const& a : stats_requested_copy)
                        results.push_back({a, this->stats(a)});
                    q.append(ox::Custom_event{[results, this] {
                        for (auto const& [asset, stats] : results)
                            this->stats_received.emit(asset, stats);
                    }});
                }
            }
        });
    }

    void launch_search_loop()
    {
        if (search_loop_.is_running())
            return;
        search_loop_.run_async([this](ox::Event_queue& q) {
            auto last_request = std::string{};  // Only search the last request.
            {  // Make a copy, b/c making requests here holds up the lock.
                auto const lock = search_requested_.lock();
                if (!search_requested_.empty())
                    last_request = search_requested_.back();
                search_requested_.clear();
            }
            if (last_request.empty())
                std::this_thread::sleep_for(std::chrono::milliseconds{300});
            else {
                auto results = this->search(last_request);
                q.append(ox::Custom_event{[results, this] {
                    this->search_results_received.emit(results);
                }});
            }
        });
    }

    [[nodiscard]] auto stats(Asset const& asset) -> Stats
    {
        auto const lock = std::lock_guard{https_socket_mtx_};
        return finnhub_.stats(asset);
    }

    [[nodiscard]] auto search(std::string const& query)
        -> std::vector<Search_result>
    {
        auto const lock = std::lock_guard{https_socket_mtx_};
        return finnhub_.search(query);
    }
};

}  // namespace crab
#endif  // CRAB_MARKETS_MARKETS_HPP
