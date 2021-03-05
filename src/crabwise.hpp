#ifndef CRAB_CRABWISE_HPP
#define CRAB_CRABWISE_HPP
#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iterator>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <termox/termox.hpp>

#include "asset_picker.hpp"
#include "format_money.hpp"
#include "markets/markets.hpp"
#include "palette.hpp"
#include "search_result.hpp"
#include "stats.hpp"

namespace crab {

class Hamburger : public ox::Button {
   public:
    Hamburger() : ox::Button{U"𝍢"} { *this | ox::pipe::fixed_width(3); };
};

class Div : public ox::Widget {
   public:
    Div()
    {
        *this | ox::pipe::wallpaper(U'│' | fg(crab::Gray)) |
            ox::pipe::fixed_width(1);
    }
};

class Asset_name : public ox::HArray<ox::HLabel, 3> {
   public:
    ox::HLabel& exchange = this->get<0>();
    ox::HLabel& base     = this->get<1>();
    ox::HLabel& quote    = this->get<2>();

   public:
    Asset_name()
    {
        exchange | ox::pipe::fixed_width(10);
        base | ox::pipe::fixed_width(6) | ox::Trait::Bold;
        quote | ox::Trait::Dim;
        *this | ox::pipe::fixed_width(21);
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
            exchange.set_text(U"Stock");
        else
            exchange.set_text(x);
    }

    void set_base(std::string const& x) { base.set_text(x); }

    void set_quote(std::string const& x) { quote.set_text(x); }
};

class Price_display : public ox::HArray<ox::HLabel, 2> {
   public:
    Price_display()
    {
        using namespace ox::pipe;
        symbol | fixed_width(3) | align_center() | ox::Trait::Dim;
    }

   public:
    /// Set the currency symbol, x is the all caps abriviation.
    void set_currency(std::string const& x) { symbol.set_text(to_symbol(x)); }

    void set_price(std::string v)
    {
        format_money(v);
        value.set_text(v);
    }

   public:
    ox::HLabel& symbol = this->get<0>();
    ox::HLabel& value  = this->get<1>();

   private:
    [[nodiscard]] static auto to_symbol(std::string const& x)
        -> ox::Glyph_string
    {
        if (x == "USD")
            return U"$";
        if (x == "ETH")
            return U"Ξ";
        if (x == "BTC")
            return U"₿";
        if (x == "XBT")
            return U"₿";
        if (x == "EUR")
            return U"€";
        if (x == "GBP")
            return U"£";
        if (x == "DAI")
            return U"◈";
        if (x == "USDC")
            return U"ᐥC";
        if (x == "USDT")
            return U"₮";
        if (x == "XRP")
            return U"✕";
        if (x == "BCH")
            return U"Ƀ";
        if (x == "BSV")
            return U"Ɓ";
        if (x == "LTC")
            return U"Ł";
        if (x == "EOS")
            return U"ε";
        if (x == "ADA")
            return U"₳";
        if (x == "XTZ")
            return U"ꜩ";
        if (x == "XMR")
            return U"ɱ";
        if (x == "ETC")
            return U"ξ";
        if (x == "MKR")
            return U"Μ";
        if (x == "ZEC")
            return U"ⓩ";
        if (x == "DOGE")
            return U"Ð";
        if (x == "REP")
            return U"Ɍ";
        if (x == "REPV2")
            return U"Ɍ";
        if (x == "STEEM")
            return U"ȿ";
        if (x == "JPY")
            return U"¥";
        if (x == "CAD")
            return U"$";
        if (x == "CHF")
            return U"₣";
        if (x == "AUD")
            return U"$";
        return "_";
    }
};

class Percent_display : public ox::HArray<ox::HLabel, 2> {
   public:
    Percent_display()
    {
        using namespace ox::pipe;
        value | align_right();
        symbol | fixed_width(3) | ox::Trait::Dim | align_center();
        symbol.set_text(U"%");
        *this | fixed_width(18);
    }

   public:
    void set_percent(double x) { value.set_text(std::to_string(x)); }

   public:
    ox::HLabel& value  = this->get<0>();
    ox::HLabel& symbol = this->get<1>();
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
                                   Div,
                                   Asset_name,
                                   Indicator,
                                   Price_display,
                                   Percent_display,
                                   ox::Widget,
                                   Price_display,
                                   Remove_btn> {
   public:
    Hamburger& hamburger            = this->get<0>();
    ox::Widget& buffer_1            = this->get<1>();
    Div& div                        = this->get<2>();
    Asset_name& name                = this->get<3>();
    Indicator& indicator            = this->get<4>();
    Price_display& last_price       = this->get<5>();
    Percent_display& percent_change = this->get<6>();
    ox::Widget& buffer_2            = this->get<7>();
    Price_display& opening_price    = this->get<8>();
    Remove_btn& remove_btn          = this->get<9>();

   public:
    Listings()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);
        last_price | fixed_width(14);
        buffer_1 | fixed_width(1);
        buffer_2 | fixed_width(4);
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

class Line : public ox::Widget {
   public:
    Line()
    {
        *this | ox::pipe::wallpaper(U'─' | fg(crab::Gray)) |
            ox::pipe::fixed_height(1);
    }
};

class Divider : public ox::HTuple<Empty, Curve, Line> {
   public:
    Divider() { *this | ox::pipe::fixed_height(1); }
};

class Ticker : public ox::Passive<ox::VPair<Listings, Divider>> {
   public:
    Listings& listings = this->first;
    Divider& divider   = this->second;

    sl::Signal<void()>& remove_me = listings.remove_btn.remove_me;

   public:
    Ticker(Asset asset, Stats stats)
        : asset_{asset}, last_price_{stats.current_price}
    {
        listings.last_price.set_currency(asset.currency.quote);
        listings.opening_price.set_currency(asset.currency.quote);
        listings.name.set(asset);

        listings.last_price.set_price(std::to_string(stats.current_price));
        this->recalculate_percent_change();

        this->update_opening_price(stats.opening_price);
    }

   public:
    void update_last_price(std::string const& value)
    {
        listings.last_price.set_price(value);
        this->recalculate_percent_change();
        auto const newer = std::stod(value);
        if (newer > last_price_)
            listings.indicator.emit_positive();
        else if (newer < last_price_)
            listings.indicator.emit_negative();
        last_price_ = newer;
    }

    void update_opening_price(double value)
    {
        opening_price_ = value;
        listings.opening_price.set_price(std::to_string(value));
        this->recalculate_percent_change();
    }

    [[nodiscard]] auto asset() const -> Asset const& { return asset_; }

   private:
    void recalculate_percent_change()
    {
        if (opening_price_ == 0.)
            return;
        listings.percent_change.set_percent(
            100. * ((last_price_ - opening_price_) / opening_price_));
    }

   private:
    Asset asset_;

    // These are only used for percentage calculation, so double is fine.
    double last_price_    = 0.;
    double opening_price_ = 0.;
};

class Ticker_list : public ox::Passive<ox::layout::Vertical<Ticker>> {
   private:
    using Base_t = ox::Passive<ox::layout::Vertical<Ticker>>;

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
    void add_ticker(Asset const& asset)
    {
        if (this->find_ticker(asset) != nullptr)
            return;  // Already Added
        markets_.subscribe(asset);
        markets_.request_stats(asset);
        auto& child = this->make_child(asset, Stats{-1., 0.});
        child.remove_me.connect(
            [this, asset = child.asset()] { this->remove_ticker(asset); });
        child.listings.hamburger.pressed.connect(
            [this, &child] { last_selected_ = &child; });
        child.listings.hamburger.install_event_filter(*this);
    }

    void remove_ticker(Asset const& asset)
    {
        auto const at = this->find_ticker(asset);
        if (at == nullptr)
            return;
        markets_.unsubscribe(asset);
        if (last_selected_ == at)
            last_selected_ = nullptr;
        this->remove_and_delete_child(at);
    }

    void update_ticker(Price price)
    {
        auto const at = this->find_ticker(price.asset);
        if (at == nullptr)
            return;
        at->update_last_price(price.value);
    }

    /// Set initial price and opening price of the given asset.
    /** No-op if asset is not in the ticker list. */
    void init_ticker(Asset const& asset, Stats const& stats)
    {
        auto const at = this->find_ticker(asset);
        if (at == nullptr)
            return;
        at->update_last_price(std::to_string(stats.current_price));
        at->update_opening_price(stats.opening_price);
    }

    /// Return list of all Assets that can be added as Tickers.
    /// Make request for async https request for Search_results.
    /** markets_.search_results_recieved emitted when finished. */
    [[nodiscard]] auto request_search(std::string const& query)
    {
        markets_.request_search(query);
    }

   protected:
    auto enable_event() -> bool override
    {
        // So loop is not launched before user input event loop.
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

   private:
    /// Return pointer to Ticker if asset matches, nullptr if can't find.
    [[nodiscard]] auto find_ticker(Asset const& asset) -> Ticker*
    {
        return this->find_child_if(
            [&](Ticker& child) { return child.asset() == asset; });
    }

   public:
    sl::Signal<void(std::vector<Search_result> const&)>&
        search_results_received = markets_.search_results_received;
};

class Column_labels : public ox::HArray<ox::HLabel, 7> {
   public:
    ox::HLabel& buffer_1       = this->get<0>();
    ox::HLabel& name           = this->get<1>();
    ox::HLabel& buffer_2       = this->get<2>();
    ox::HLabel& last_price     = this->get<3>();
    ox::HLabel& percent_change = this->get<4>();
    ox::HLabel& buffer_3       = this->get<5>();
    ox::HLabel& opening_price  = this->get<6>();

   public:
    Column_labels()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);
        name.set_text(U"Asset" | ox::Trait::Bold);
        name | fixed_width(21);
        last_price.set_text(U"  Last Price" | ox::Trait::Bold);
        last_price | fixed_width(14);
        percent_change.set_text(U"Change    " | ox::Trait::Bold);
        percent_change | align_right() | fixed_width(18);
        opening_price.set_text(U"  Opening Price" | ox::Trait::Bold);

        buffer_1 | fixed_width(5);
        buffer_2 | fixed_width(3);
        buffer_3 | fixed_width(4);
    }
};

// TODO change this into something else, just something small.
class Status_bar : public ox::HTuple<ox::Widget, ox::Tile, ox::HLabel> {
   private:
    // ox::Widget& arrow     = this->get<0>();
    // ox::Tile& buffer      = this->get<1>();
    // ox::HLabel& text_area = this->get<2>();

   public:
    Status_bar()
    {
        *this | ox::pipe::fixed_height(1) | ox::pipe::descendants() |
            bg(crab::Foreground) | fg(crab::Background);
        // arrow | ox::pipe::wallpaper(U'├') | ox::pipe::fixed_width(1) |
        //     bg(ox::Color::Foreground) | fg(ox::Color::Background);
        // buffer | bg(ox::Color::Foreground) | fg(ox::Color::Background);
        // text_area.set_text(U"Status");
        // text_area | bg(ox::Color::Foreground) | fg(ox::Color::Background);
    }
};

class App_space
    : public ox::HTuple<
          Asset_picker,
          ox::VTuple<Column_labels,
                     ox::HTuple<ox::VTuple<Line, Ticker_list, ox::Widget>,
                                ox::VScrollbar>,
                     Status_bar>> {
   public:
    Asset_picker& asset_picker = this->get<0>();
    Ticker_list& ticker_list   = this->get<1>().get<1>().get<0>().get<1>();
    ox::Widget& buffer         = this->get<1>().get<1>().get<0>().get<2>();
    Status_bar& status_bar     = this->get<1>().get<2>();
    ox::VScrollbar& scrollbar  = this->get<1>().get<1>().get<1>();

   public:
    App_space()
    {
        link(scrollbar, ticker_list);
        buffer.install_event_filter(scrollbar);

        asset_picker.search_results.selected.connect(
            [this](Asset const& asset) {
                this->ticker_list.add_ticker(asset);
            });

        asset_picker.search_input.search_request.connect(
            [this](std::string const& s) {
                asset_picker.search_results.clear_results();
                this->ticker_list.request_search(s);
            });

        ticker_list.search_results_received.connect(
            [this](std::vector<Search_result> const& results) {
                asset_picker.search_results.clear_results();
                for (auto const& search_result : results)
                    asset_picker.search_results.add_result(search_result);
            });
    }
};

class Crabwise : public ox::VTuple<ox::Titlebar, App_space> {
   public:
    ox::Titlebar& titlebar = this->get<0>();
    App_space& app_space   = this->get<1>();

   public:
    /// Reads \p init_filename and adds currencies.
    Crabwise(std::string const& init_filename)
    {
        titlebar.title.set_text(U"CrabWise" | ox::Trait::Bold);
        ox::Terminal::set_palette(crab::palette);
        auto const init_assets = parse_init_file(init_filename);
        for (Asset const& a : init_assets)
            app_space.ticker_list.add_ticker(a);
    }

   private:
    /// std::to_upper() each letter, returns ref to passed in modified string.
    [[nodiscard]] static auto upper(std::string& x) -> std::string&
    {
        for (char& c : x)
            c = std::toupper(c);
        return x;
    }

    /// Return nullopt if not an Asset
    /** returns empty Asset and exchange name, or empty exchange name and full
     *  Asset. */
    [[nodiscard]] static auto parse_line(std::string const& line)
        -> std::pair<std::string, Currency_pair>
    {
        auto ss    = std::istringstream{line};
        auto first = std::string{};

        assert((bool)ss);
        ss >> first;

        if (first[first.size() - 1] == ':') {
            first.pop_back();
            return {upper(first), {"", ""}};
        }
        if (first.front() == '#')
            return {"", {"", ""}};

        auto quote = std::string{};
        ss >> quote;
        return {"", {upper(first), upper(quote)}};
    }

    /// Return true if string is all space characters or is empty.
    [[nodiscard]] static auto all_is_space(std::string const& x) -> bool
    {
        for (char c : x)
            if (!std::isspace(c))
                return false;
        return true;
    }

    // Exchange:
    //     Base Quote
    //     Base Quote
    //     Base Quote
    //
    // Stock:
    //     Symbol
    //     Symbol
    // # Comment
    [[nodiscard]] static auto parse_init_file(std::string const& filename)
        -> std::vector<Asset>
    {
        auto result           = std::vector<Asset>{};
        auto file             = std::ifstream{filename};
        auto line             = std::string{};
        auto current_exchange = std::string{};
        while (std::getline(file, line, '\n')) {
            if (all_is_space(line))
                continue;
            auto const [exchange, currency] = parse_line(line);
            if (exchange.empty() && currency.base.empty())
                continue;
            if (exchange.empty()) {
                if (current_exchange == "STOCK")
                    result.push_back({"", {currency.base, "USD"}});
                else
                    result.push_back({current_exchange, currency});
            }
            else
                current_exchange = exchange;
        }
        return result;
    }
};

}  // namespace crab
#endif  // CRAB_CRABWISE_HPP
