#ifndef CRAB_TICKER_LIST_HPP
#define CRAB_TICKER_LIST_HPP
#include <cassert>
#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include <termox/common/filter_view.hpp>
#include <termox/termox.hpp>

#include "amount_display.hpp"
#include "asset.hpp"
#include "format_money.hpp"
#include "line.hpp"
#include "markets/markets.hpp"
#include "palette.hpp"
#include "percent_display.hpp"
#include "price.hpp"
#include "price_display.hpp"
#include "price_edit.hpp"
#include "quantity_edit.hpp"
#include "stats.hpp"

namespace crab {

class Hamburger : public ox::Button {
   public:
    Hamburger() : ox::Button{U"𝍢"} { *this | ox::pipe::fixed_width(3); };
};

class Asset_name : public ox::HArray<ox::HLabel, 4> {
   public:
    ox::HLabel& buffer = this->get<0>();
    ox::HLabel& base   = this->get<1>();
    ox::HLabel& market = this->get<2>();
    ox::HLabel& quote  = this->get<3>();

   public:
    Asset_name()
    {
        using namespace ox::pipe;
        buffer | fixed_width(1);
        market | fixed_width(10);
        base | fixed_width(6) | ox::Trait::Bold;
        quote | ox::Trait::Dim;
        *this | fixed_width(22);
    }

   public:
    void set(Asset const& asset)
    {
        this->set_exchange(asset.exchange);
        this->set_base(asset.currency.base);
        this->set_quote(asset.currency.quote);
    }

   private:
    void set_exchange(std::string const& x)
    {
        if (x.empty())
            market.set_text(U"Stock");
        else
            market.set_text(x);
    }

    void set_base(std::string const& x) { base.set_text(x); }

    void set_quote(std::string const& x) { quote.set_text(x); }
};

class Indicator : public ox::Notify_light {
   public:
    Indicator() { this->set_duration(ox::Notify_light::Duration_t{220}); }

   public:
    void emit_positive()
    {
        this->Notify_light::set_display(
            {ox::Color::Green, ox::Color::Background});
        this->Notify_light::emit();
    }

    void emit_negative()
    {
        this->Notify_light::set_display(
            {ox::Color::Red, ox::Color::Background});
        this->Notify_light::emit();
    }
};

class Remove_btn : public ox::Button {
   public:
    sl::Signal<void()>& remove_me = this->Button::pressed;

   public:
    Remove_btn() : Button{"x" | ox::Trait::Bold | ox::Trait::Dim}
    {
        *this | ox::pipe::fixed_height(1) | ox::pipe::fixed_width(3);
    }
};

class Listings : public ox::HTuple<Hamburger,
                                   ox::Widget,
                                   VLine,
                                   Asset_name,
                                   Indicator,
                                   Price_display,
                                   Percent_display,
                                   ox::Widget,
                                   Price_display,
                                   ox::Widget,
                                   Quantity_edit,
                                   ox::Widget,
                                   Aligned_price_display,
                                   ox::Widget,
                                   Price_edit,
                                   ox::Widget,
                                   Aligned_price_display,
                                   ox::Widget,
                                   Aligned_price_display,
                                   ox::Widget,
                                   VLine,
                                   Remove_btn> {
   public:
    Hamburger& hamburger            = this->get<0>();
    ox::Widget& buffer_1            = this->get<1>();
    VLine& div1                     = this->get<2>();
    Asset_name& name                = this->get<3>();
    Indicator& indicator            = this->get<4>();
    Price_display& last_price       = this->get<5>();
    Percent_display& percent_change = this->get<6>();
    ox::Widget& buffer_2            = this->get<7>();
    Price_display& last_close       = this->get<8>();
    ox::Widget& buffer_3            = this->get<9>();
    Quantity_edit& quantity         = this->get<10>();
    ox::Widget& buffer_4            = this->get<11>();
    Aligned_price_display& value    = this->get<12>();
    ox::Widget& buffer_5            = this->get<13>();
    Price_edit& cost_basis          = this->get<14>();
    ox::Widget& buffer_6            = this->get<15>();
    Aligned_price_display& open_pl  = this->get<16>();
    ox::Widget& buffer_7            = this->get<17>();
    Aligned_price_display& daily_pl = this->get<18>();
    ox::Widget& buffer_8            = this->get<19>();
    VLine& div2                     = this->get<20>();
    Remove_btn& remove_btn          = this->get<21>();

   public:
    Listings()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);
        buffer_1 | fixed_width(1);
        last_price | fixed_width(12);
        percent_change | fixed_width(12);
        buffer_2 | fixed_width(2);
        last_close | fixed_width(13);
        buffer_3 | fixed_width(3);
        quantity | fixed_width(12);
        buffer_4 | fixed_width(2);
        value | fixed_width(14);
        buffer_5 | fixed_width(1);
        cost_basis | fixed_width(13);
        buffer_6 | fixed_width(1);
        open_pl | fixed_width(14);
        buffer_7 | fixed_width(1);
        daily_pl | fixed_width(14);
    }

   public:
    [[nodiscard]] auto current_stats() const -> Stats
    {
        return {last_price.amount.as_double(), last_close.amount.as_double()};
    }
};

class Empty : public ox::Widget {
   public:
    Empty() { *this | ox::pipe::fixed_width(4); }
};

class Curve : public ox::Widget {
   public:
    Curve()
    {
        *this | ox::pipe::fixed_width(1) |
            ox::pipe::wallpaper(U'╰' | fg(crab::Gray));
    }
};

class Bottom_line : public ox::HTuple<Empty, Curve, HLine> {
   public:
    Bottom_line() { *this | ox::pipe::fixed_height(1); }
};

class Ticker : public ox::Passive<ox::VPair<Listings, Bottom_line>> {
   public:
    Listings& listings       = this->first;
    Bottom_line& bottom_line = this->second;

    sl::Signal<void()>& remove_me = listings.remove_btn.remove_me;

   public:
    Ticker(Asset asset, Stats stats, double quantity, double cost_basis)
        : asset_{asset}, last_price_{stats.last_price}
    {
        listings.value.amount.amount_updated.connect([this](double) {
            this->reset_open_pl();
            this->reset_daily_pl();
        });
        listings.cost_basis.amount.quantity_updated.connect(
            [this](double) { this->reset_open_pl(); });
        listings.quantity.quantity_updated.connect([this](double quant) {
            this->update_value(quant, listings.last_price.amount.as_double());
        });
        listings.last_close.amount.amount_updated.connect([this](double) {
            this->recalculate_percent_change();
            this->reset_daily_pl();
        });

        listings.last_price.currency.set(asset.currency.quote);
        listings.last_close.currency.set(asset.currency.quote);
        listings.value.currency.set(asset.currency.quote);
        if (is_USD_like(asset_.currency.quote)) {
            listings.value.amount.round_to_hundredths(true);
            listings.value.amount.set_offset(8);
            listings.open_pl.amount.round_to_hundredths(true);
            listings.open_pl.amount.set_offset(8);
            listings.daily_pl.amount.round_to_hundredths(true);
            listings.daily_pl.amount.set_offset(8);
        }
        listings.name.set(asset);

        listings.last_price.amount.set(stats.last_price);
        this->recalculate_percent_change();

        listings.quantity.initialize(quantity);
        listings.cost_basis.amount.initialize(cost_basis);

        this->update_last_close(stats.last_close);

        listings.cost_basis.currency.set(asset.currency.quote);
        listings.open_pl.currency.set(asset.currency.quote);
        listings.daily_pl.currency.set(asset.currency.quote);
    }

   public:
    void update_last_price(std::string const& value)
    {
        listings.last_price.amount.set(value);
        auto count       = std::size_t{0};
        auto const newer = std::stod(value, &count);
        assert(count == value.size());
        auto const older = last_price_;
        last_price_      = newer;
        if (newer > older)
            listings.indicator.emit_positive();
        else if (newer < older)
            listings.indicator.emit_negative();
        this->recalculate_percent_change();
        this->update_value(listings.quantity.quantity(),
                           listings.last_price.amount.as_double());
    }

    void update_last_close(double value)
    {
        last_close_ = value;
        listings.last_close.amount.set(value);
        this->recalculate_percent_change();
    }

    [[nodiscard]] auto asset() const -> Asset const& { return asset_; }

    [[nodiscard]] auto quantity() const -> double
    {
        return listings.quantity.quantity();
    }

    [[nodiscard]] auto cost_basis() const -> double
    {
        return listings.cost_basis.amount.quantity();
    }

   private:
    [[nodiscard]] static auto calc_value(double quantity, double last_price)
        -> double
    {
        return quantity * last_price;
    }

   private:
    void recalculate_percent_change()
    {
        if (last_close_ == 0.)
            return;
        listings.percent_change.set_percent(
            100. * ((last_price_ - last_close_) / last_close_));
    }

    void update_value(double quantity, double last_price)
    {
        listings.value.amount.set(calc_value(quantity, last_price));
    }

    void reset_open_pl()
    {
        auto const initial_value = listings.quantity.quantity() *
                                   listings.cost_basis.amount.quantity();
        auto const current_value = listings.value.amount.as_double();
        listings.open_pl.amount.set(current_value - initial_value);
    }

    void reset_daily_pl()
    {
        auto const diff = listings.last_price.amount.as_double() -
                          listings.last_close.amount.as_double();
        auto const quantity = listings.quantity.quantity();
        listings.daily_pl.amount.set(quantity * diff);
    }

   private:
    Asset asset_;

    // These are only used for percentage calculation, so double is fine.
    double last_price_ = 0.;
    double last_close_ = 0.;
};

class Ticker_list : public ox::Passive<ox::layout::Vertical<Ticker>> {
   private:
    using Base_t = ox::Passive<ox::layout::Vertical<Ticker>>;

   public:
    // Sends quote currency and sum
    sl::Signal<void(std::string const&, double)> value_total_updated;
    sl::Signal<void(std::string const&, double)> open_pl_total_updated;
    sl::Signal<void(std::string const&, double)> daily_pl_total_updated;

   private:
    /// Return pointer to first Ticker if asset matches, nullptr if can't find.
    [[nodiscard]] auto find_ticker(Asset const& asset) -> Ticker*
    {
        return this->find_child_if(
            [&](Ticker& child) { return child.asset() == asset; });
    }

    /// Returns a filtered view of all Tickers holding the given \p asset.
    [[nodiscard]] auto asset_ticker_view(Asset const& asset)
    {
        return ox::Owning_filter_view{
            this->get_children(),
            [asset](auto const& child) { return child.asset() == asset; }};
    }

   public:
    Ticker_list()
    {
        markets_.price_update.connect(
            [this](Price const& p) { this->update_ticker(p); });
        markets_.stats_received.connect(
            [this](Asset const& asset, Stats const& stats) {
                this->init_ticker(asset, stats);
            });
    }

    ~Ticker_list() { markets_.shutdown(); }

   public:
    void add_ticker(Asset const& asset, double quantity, double cost_basis)
    {
        Ticker* const existing = this->find_ticker(asset);
        auto stats             = Stats{-1., 0.};
        if (existing == nullptr) {
            markets_.subscribe(asset);
            markets_.request_stats(asset);
        }
        else
            stats = existing->listings.current_stats();

        auto& child = this->make_child(asset, stats, quantity, cost_basis);
        child.remove_me.connect([this, &child_ref = child] {
            auto const quote = child_ref.asset().currency.quote;
            this->remove_ticker(child_ref);
            value_total_updated.emit(quote, this->value_sum(quote));
            open_pl_total_updated.emit(quote, this->open_pl_sum(quote));
            daily_pl_total_updated.emit(quote, this->daily_pl_sum(quote));
        });
        child.listings.hamburger.pressed.connect(
            [this, &child] { last_selected_ = &child; });
        child.listings.hamburger.install_event_filter(*this);
        child.listings.value.amount.amount_updated.connect(
            [quote = child.asset().currency.quote, this](double) {
                value_total_updated.emit(quote, this->value_sum(quote));
            });
        child.listings.open_pl.amount.amount_updated.connect(
            [quote = child.asset().currency.quote, this](double) {
                open_pl_total_updated.emit(quote, this->open_pl_sum(quote));
            });
        child.listings.daily_pl.amount.amount_updated.connect(
            [quote = child.asset().currency.quote, this](double) {
                daily_pl_total_updated.emit(quote, this->daily_pl_sum(quote));
            });
    }

    void remove_ticker(Ticker& ticker_ref)
    {
        auto const asset = ticker_ref.asset();
        if (last_selected_ == &ticker_ref)
            last_selected_ = nullptr;
        this->remove_and_delete_child(&ticker_ref);
        if (this->find_ticker(asset) == nullptr)  // Last ticker of this asset.
            markets_.unsubscribe(asset);
    }

    /// Update the last price of each Ticker with the Asset within \p price.
    void update_ticker(Price price)
    {
        for (Ticker& child : asset_ticker_view(price.asset))
            child.update_last_price(price.value);
    }

    /// Set initial price and opening price of the given asset.
    /** No-op if asset is not in the ticker list. */
    void init_ticker(Asset const& asset, Stats const& stats)
    {
        auto const current_str = std::to_string(stats.last_price);
        for (Ticker& child : asset_ticker_view(asset)) {
            child.update_last_close(stats.last_close);
            child.update_last_price(current_str);
        }
    }

    /// Return list of all Assets that can be added as Tickers.
    /** Makes async request via https for Search_results.
     *  markets_.search_results_recieved emitted when finished. */
    [[nodiscard]] auto request_search(std::string const& query)
    {
        markets_.request_search(query);
    }

   protected:
    auto enable_event() -> bool override
    {
        // So that loop is not launched before user input event loop.
        markets_.launch_streams();
        return Base_t::enable_event();
    }

    auto mouse_move_event_filter(Widget& receiver, ox::Mouse const& m)
        -> bool override
    {
        if (m.button != ox::Mouse::Button::Left || last_selected_ == nullptr)
            return false;
        auto* ticker       = receiver.parent()->parent();
        auto const index_a = this->find_child_position(ticker);
        auto const index_b = this->find_child_position(last_selected_);
        if (index_a == std::size_t(-1) || index_b == std::size_t(-1))
            return false;
        this->swap_children(index_a, index_b);
        return true;
    }

   private:
    Markets markets_;
    Ticker* last_selected_ = nullptr;

   public:
    sl::Signal<void(std::vector<Search_result> const&)>&
        search_results_received = markets_.search_results_received;

   private:
    /// Returns a lambda that takes a Ticker and returns true if \p q matches.
    [[nodiscard]] static auto matches_quote(std::string const& q)
    {
        return [q](Ticker const& t) { return t.asset().currency.quote == q; };
    }

    /// Return Container view of all Ticker children that have the given quote.
    [[nodiscard]] auto quote_ticker_view(std::string const& quote) const
    {
        return ox::Owning_filter_view{this->get_children(),
                                      matches_quote(quote)};
    }

    /// Get the Value column sum of all Tickers with \p quote_currency.
    [[nodiscard]] auto value_sum(std::string const& quote_currency) const
        -> double
    {
        auto sum = 0.;
        for (Ticker const& child : quote_ticker_view(quote_currency))
            sum += child.listings.value.amount.as_double();
        return sum;
    }

    /// Get the Open P&L column sum of all Tickers with \p quote_currency.
    [[nodiscard]] auto open_pl_sum(std::string const& quote_currency) const
        -> double
    {
        auto sum = 0.;
        for (Ticker const& child : quote_ticker_view(quote_currency))
            sum += child.listings.open_pl.amount.as_double();
        return sum;
    }

    /// Get the Daily P&L column sum of all Tickers with \p quote_currency.
    [[nodiscard]] auto daily_pl_sum(std::string const& quote_currency) const
        -> double
    {
        auto sum = 0.;
        for (Ticker const& child : quote_ticker_view(quote_currency))
            sum += child.listings.daily_pl.amount.as_double();
        return sum;
    }
};

class Column_labels : public ox::HArray<ox::HLabel, 20> {
   public:
    ox::HLabel& buffer_1       = this->get<0>();
    ox::HLabel& base           = this->get<1>();
    ox::HLabel& market         = this->get<2>();
    ox::HLabel& quote          = this->get<3>();
    ox::HLabel& buffer_2       = this->get<4>();
    ox::HLabel& last_price     = this->get<5>();
    ox::HLabel& percent_change = this->get<6>();
    ox::HLabel& buffer_3       = this->get<7>();
    ox::HLabel& last_close     = this->get<8>();
    ox::HLabel& buffer_4       = this->get<9>();
    ox::HLabel& quantity       = this->get<10>();
    ox::HLabel& buffer_5       = this->get<11>();
    ox::HLabel& value          = this->get<12>();
    ox::HLabel& buffer_6       = this->get<13>();
    ox::HLabel& cost_basis     = this->get<14>();
    ox::HLabel& buffer_7       = this->get<15>();
    ox::HLabel& open_pl        = this->get<16>();
    ox::HLabel& buffer_8       = this->get<17>();
    ox::HLabel& daily_pl       = this->get<18>();
    ox::HLabel& buffer         = this->get<19>();

   public:
    Column_labels()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);
        buffer_1 | fixed_width(5);

        // TODO use pipe::text

        base.set_text(U"Base" | ox::Trait::Bold);
        base | fixed_width(6);
        market.set_text(U" Market");
        market | fixed_width(9);
        quote.set_text(U"  Quote");
        quote | fixed_width(7);

        buffer_2 | fixed_width(2);
        last_price.set_text(U" Last Price" | ox::Trait::Bold);
        last_price | fixed_width(12);
        percent_change.set_text(U"Change " | ox::Trait::Bold);
        percent_change | align_right() | fixed_width(12);
        buffer_3 | fixed_width(2);
        last_close.set_text(U" Last Close" | ox::Trait::Bold);
        last_close | fixed_width(13);
        buffer_4 | fixed_width(3);
        quantity.set_text(U"Quantity" | ox::Trait::Bold);
        quantity | fixed_width(12);
        buffer_5 | fixed_width(2);
        value.set_text(U"Value" | ox::Trait::Bold);
        value | align_right() | fixed_width(14);
        buffer_6 | fixed_width(1);
        cost_basis.set_text(U"   Cost Basis" | ox::Trait::Bold);
        cost_basis | fixed_width(13);
        buffer_7 | fixed_width(1);
        open_pl.set_text(U"Open P&L" | ox::Trait::Bold);
        open_pl | align_right() | fixed_width(14);
        buffer_8 | fixed_width(1);
        daily_pl.set_text(U"Daily P&L" | ox::Trait::Bold);
        daily_pl | align_right() | fixed_width(14);
    }
};

}  // namespace crab
#endif  // CRAB_TICKER_LIST_HPP
