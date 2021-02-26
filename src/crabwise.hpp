#ifndef CRAB_CRABWISE_HPP
#define CRAB_CRABWISE_HPP
#include <string>

#include <termox/painter/palette/en4.hpp>
#include <termox/termox.hpp>

#include "format_money.hpp"
#include "markets/coinbase.hpp"
#include "termox/widget/pipe.hpp"
#include "termox/widget/widgets/notify_light.hpp"

namespace crab {

class Hamburger : public ox::HLabel {
   public:
    Hamburger() : ox::HLabel{U"𝍢"}
    {
        *this | ox::pipe::align_center() | ox::pipe::fixed_width(3);
    };
};

class Div : public ox::Widget {
   public:
    Div() { *this | ox::pipe::wallpaper(U'│') | ox::pipe::fixed_width(1); }
};

class Name : public ox::HArray<ox::HLabel, 2> {
   public:
    ox::HLabel& base  = this->get<0>();
    ox::HLabel& quote = this->get<1>();

   public:
    Name()
    {
        base | ox::pipe::fixed_width(8) | ox::Trait::Bold;
        quote | ox::Trait::Dim;
        *this | ox::pipe::fixed_width(16);
    }

   public:
    void set_base(std::string const& x) { base.set_text(x); }

    void set_quote(std::string const& x) { quote.set_text(x); }
};

class Price_display : public ox::HArray<ox::HLabel, 2> {
   public:
    Price_display()
    {
        using namespace ox::pipe;
        symbol | fixed_width(3) | align_center();
        symbol.set_text(U"$" | ox::Trait::Dim);
    }

   public:
    void set_price(std::string v)
    {
        format_money(v);
        value.set_text(v);
    }

   public:
    ox::HLabel& symbol = this->get<0>();
    ox::HLabel& value  = this->get<1>();
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
    // TODO formatting
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
                                   Name,
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
    Name& name                      = this->get<3>();
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
    Curve() { *this | ox::pipe::fixed_width(1) | ox::pipe::wallpaper(U'╰'); }
};

class Line : public ox::Widget {
   public:
    Line() { *this | ox::pipe::wallpaper(U'─') | ox::pipe::fixed_height(1); }
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
    Ticker(Currency_pair currency,
           std::string const& last_price,
           std::string const& opening_price)
    {
        listings.name.set_base(currency.base);
        listings.name.set_quote(currency.quote);
        currency_pair_ = currency;
        this->update_last_price(last_price);
        this->update_opening_price(opening_price);
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

    void update_opening_price(std::string const& value)
    {
        opening_price_ = std::stod(value);
        listings.opening_price.set_price(value);
        this->recalculate_percent_change();
    }

    [[nodiscard]] auto currency_pair() const -> Currency_pair const&
    {
        return currency_pair_;
    }

   private:
    void recalculate_percent_change()
    {
        if (opening_price_ == 0.)
            return;
        listings.percent_change.set_percent(
            100. * ((last_price_ - opening_price_) / opening_price_));
    }

   private:
    // These are only used for percentage calculation, so double is fine.
    double last_price_    = 0.;
    double opening_price_ = 0.;

    Currency_pair currency_pair_;
};

class Ticker_list : public ox::Passive<ox::layout::Vertical<Ticker>> {
   private:
    using Base_t = ox::Passive<ox::layout::Vertical<Ticker>>;

   public:
    Ticker_list()
    {
        coinbase_.make_https_connection();
        this->subscribe_to_all();
    }

    ~Ticker_list() { coinbase_.disconnect_https(); }

   public:
    void add_ticker(Currency_pair currency)
    {
        if (this->find_ticker(currency) != nullptr)
            return;
        auto& child =
            this->make_child(currency, coinbase_.current_price(currency).value,
                             coinbase_.opening_price(currency).value);
        child.remove_me.connect(
            [this, cp = child.currency_pair()] { this->remove_ticker(cp); });
    }

    void remove_ticker(Currency_pair currency)
    {
        auto const at = this->find_ticker(currency);
        if (at == nullptr)
            return;
        this->remove_and_delete_child(at);
    }

    void update_ticker(Price price)
    {
        auto const at = this->find_ticker(price.currency);
        if (at == nullptr)
            return;
        at->update_last_price(price.value);
    }

   protected:
    auto enable_event() -> bool override
    {
        // So loop is not launched before user input event loop.
        if (!coinbase_loop_.is_running()) {
            coinbase_loop_.run_async([&](ox::Event_queue& q) {
                auto const last_price = coinbase_.read();
                q.append(
                    ox::Custom_event{[=] { this->update_ticker(last_price); }});
            });
        }
        return Base_t::enable_event();
    }

   public:
    Coinbase coinbase_;
    ox::Event_loop coinbase_loop_;

   private:
    void subscribe_to_all()
    {
        auto const& pairs = coinbase_.currency_pairs();
        for (Currency_pair const& c : pairs)
            coinbase_.subscribe(c);
    }

    /// Return pointer to Ticker if currency matches, nullptr if can't find.
    [[nodiscard]] auto find_ticker(Currency_pair const& currency) -> Ticker*
    {
        return this->find_child_if(
            [&](Ticker& child) { return child.currency_pair() == currency; });
    }
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
        name.set_text(U"Name" | ox::Trait::Bold);
        name | fixed_width(16);
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

class App_space
    : public ox::HPair<ox::VTuple<Column_labels, Line, Ticker_list, ox::Widget>,
                       ox::VScrollbar> {
   public:
    ox::VScrollbar& scrollbar = this->second;
    Ticker_list& ticker_list  = this->first.get<2>();
    ox::Widget& buffer        = this->first.get<3>();

   public:
    App_space()
    {
        link(scrollbar, ticker_list);
        buffer.install_event_filter(scrollbar);
    }
};

class Status_bar : public ox::HLabel {
   public:
    Status_bar()
    {
        this->set_text(U"Status");
        *this | bg(ox::Color::Foreground) | fg(ox::Color::Background);
    }
};

class Crabwise : public ox::VTuple<ox::Titlebar, App_space, Status_bar> {
   public:
    ox::Titlebar& titlebar = this->get<0>();
    App_space& app_space   = this->get<1>();
    Status_bar& status_bar = this->get<2>();

   public:
    Crabwise()
    {
        titlebar.title.set_text(U"CrabWise");
        ox::Terminal::set_palette(ox::en4::palette);
        app_space.ticker_list.add_ticker({"BTC", "USD"});
        app_space.ticker_list.add_ticker({"ALGO", "USD"});
        app_space.ticker_list.add_ticker({"XLM", "USD"});
        app_space.ticker_list.add_ticker({"ETH", "USD"});
        app_space.ticker_list.add_ticker({"NU", "USD"});
        app_space.ticker_list.add_ticker({"COMP", "USD"});
        app_space.ticker_list.add_ticker({"MKR", "USD"});
        app_space.ticker_list.add_ticker({"CGLD", "USD"});
        app_space.ticker_list.add_ticker({"GRT", "USD"});
        app_space.ticker_list.add_ticker({"AAVE", "USD"});
    }
};

}  // namespace crab
#endif  // CRAB_CRABWISE_HPP
