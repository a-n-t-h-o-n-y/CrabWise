#ifndef CRAB_COIN_DRAWER_HPP
#define CRAB_COIN_DRAWER_HPP
#include <cassert>
#include <cctype>
#include <vector>

#include <termox/termox.hpp>

#include "asset.hpp"
#include "currency_pair.hpp"

namespace crab {

class Currency_list
    : public ox::Passive<ox::layout::Vertical<ox::HThin_button>> {
   public:
    sl::Signal<void(Currency_pair const&)> currency_selected;

   public:
    void add_currency(Currency_pair const& currency)
    {
        auto& child = this->make_child(format_currency_name(currency));
        child.pressed.connect(
            [this, currency] { this->currency_selected(currency); });
    }

   private:
    [[nodiscard]] static auto format_currency_name(
        Currency_pair const& currency) -> ox::Glyph_string
    {
        auto result = (ox::Glyph_string{currency.base} | ox::Trait::Bold);
        if (currency.base.size() <= 4)
            result.append(std::string(4 - currency.base.size(), ' '));
        result.append(U" / ").append(currency.quote);
        return result;
    }
};

class Exchange_listing : public ox::VAccordion<Currency_list> {
   private:
    using Base_t = ox::VAccordion<Currency_list>;

    // ox::VScrollbar& scrollbar    = this->wrapped().first;
    Currency_list& currency_list = this->wrapped();

   public:
    sl::Signal<void(Asset const&)> asset_selected;

   public:
    Exchange_listing(std::string const& exchange)
        : Base_t{{reformat_exchange_name(get_exchange_name(exchange))}},
          exchange_{exchange}
    {
        // link(scrollbar, currency_list);
        currency_list.height_policy.policy_updated.connect(
            [this] { this->height_policy = currency_list.height_policy; });
        currency_list.currency_selected.connect(
            [this](Currency_pair const& currency) {
                asset_selected.emit({this->exchange(), currency});
            });
    }

   public:
    auto exchange() const -> std::string const& { return exchange_; }

    void add_currency(Currency_pair const& currency)
    {
        currency_list.add_currency(currency);
    }

   private:
    std::string exchange_;

   private:
    [[nodiscard]] static auto reformat_exchange_name(
        std::string const& exchange_name) -> std::string
    {
        assert(!exchange_name.empty());
        auto result = exchange_name;
        for (char& c : result)
            c = std::tolower(c);
        result.front() = std::toupper(result.front());
        return result;
    }

   private:
    /// Return "Stocks" if exchange is empty.
    [[nodiscard]] static auto get_exchange_name(std::string const& exchange)
        -> std::string
    {
        return exchange.empty() ? "Stocks" : exchange;
    }
};

class Exchange_listings : public ox::layout::Vertical<Exchange_listing> {
   public:
    sl::Signal<void(Asset const&)> asset_selected;

   public:
    Exchange_listings() { *this | ox::pipe::fixed_width(18); }

   public:
    void add_asset(Asset const& asset)
    {
        auto* child = this->find_child_if([&](auto const& child) {
            return child.exchange() == asset.exchange;
        });
        if (child != nullptr)
            child->add_currency(asset.currency);
        else {
            auto& child = this->make_child(asset.exchange);
            child.asset_selected.connect(
                [this](Asset const& asset) { asset_selected.emit(asset); });
            child.add_currency(asset.currency);
        }
    }
};

class Coin_drawer
    : public ox::HAccordion<ox::VPair<Exchange_listings, ox::Widget>,
                            ox::Bar_position::Last> {
   private:
    using Base_t = ox::HAccordion<ox::VPair<Exchange_listings, ox::Widget>,
                                  ox::Bar_position::Last>;

   public:
    Coin_drawer() : Base_t{{U"Coin Drawer", ox::Align::Center}}
    {
        this->wrapped() | ox::pipe::fixed_width(18);
    }

   public:
    sl::Signal<void(Asset const&)>& asset_selected =
        this->wrapped().first.asset_selected;

   public:
    void add_asset(Asset const& asset)
    {
        this->wrapped().first.add_asset(asset);
    }
};

}  // namespace crab
#endif  // CRAB_COIN_DRAWER_HPP
