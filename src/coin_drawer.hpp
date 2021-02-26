#ifndef CRAB_COIN_DRAWER_HPP
#define CRAB_COIN_DRAWER_HPP
#include <vector>

#include <termox/termox.hpp>

#include "currency_pair.hpp"
#include "termox/widget/widgets/button.hpp"
#include "termox/widget/widgets/scrollbar.hpp"

namespace crab {

class Currency_list
    : public ox::Passive<ox::layout::Vertical<ox::HThin_button>> {
   public:
    sl::Signal<void(Currency_pair const&)> currency_selected;

   public:
    void add_currencies(std::vector<Currency_pair> const& currencies)
    {
        for (auto const& currency : currencies) {
            auto& child = this->make_child(format_currency_name(currency));
            child.pressed.connect(
                [this, currency] { this->currency_selected(currency); });
        }
    }

   private:
    [[nodiscard]] static auto format_currency_name(
        Currency_pair const& currency) -> ox::Glyph_string
    {
        return (ox::Glyph_string{currency.base} | ox::Trait::Bold)
            .append(std::string(4 - currency.base.size(), ' '))
            .append(U" / ")
            .append(currency.quote);
    }
};

class Scrolling_currency_list
    : public ox::HPair<ox::VScrollbar, Currency_list> {
   public:
    ox::VScrollbar& scrollbar    = this->first;
    Currency_list& currency_list = this->second;

   public:
    Scrolling_currency_list()
    {
        link(scrollbar, currency_list);
        currency_list.height_policy.policy_updated.connect(
            [this] { this->height_policy = currency_list.height_policy; });
    }
};

class Coinbase_listings : public ox::VAccordion<Scrolling_currency_list> {
   private:
    using Base_t = ox::VAccordion<Scrolling_currency_list>;

   public:
    Currency_list& currency_list = this->wrapped().currency_list;

   public:
    Coinbase_listings() : Base_t{{U"Coinbase"}} { this->expand(); }
};

class Coin_listings : public ox::VTuple<Coinbase_listings, ox::Widget> {
   public:
    Coinbase_listings& coinbase_listings = this->get<0>();
    ox::Widget& buffer                   = this->get<1>();

   public:
    Coin_listings() { *this | ox::pipe::fixed_width(18); }
};

class Coin_drawer
    : public ox::HAccordion<Coin_listings, ox::Bar_position::Last> {
   private:
    using Base_t = ox::HAccordion<Coin_listings, ox::Bar_position::Last>;

   public:
    Coin_listings& coin_listings = this->wrapped();

   public:
    sl::Signal<void(Currency_pair const&)>& currency_selected =
        coin_listings.coinbase_listings.currency_list.currency_selected;

   public:
    Coin_drawer() : Base_t{{U"Coin Drawer", ox::Align::Center}} {}
};

}  // namespace crab
#endif  // CRAB_COIN_DRAWER_HPP
