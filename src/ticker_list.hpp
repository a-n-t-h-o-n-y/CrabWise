#ifndef CRAB_TICKER_LIST_HPP
#define CRAB_TICKER_LIST_HPP
#include <cassert>
#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include <termox/termox.hpp>

#include "asset.hpp"
#include "format_money.hpp"
#include "markets/markets.hpp"
#include "palette.hpp"
#include "price.hpp"
#include "stats.hpp"
#include "termox/system/key.hpp"
#include "termox/widget/pipe.hpp"

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

class Asset_name : public ox::HArray<ox::HLabel, 4> {
   public:
    ox::HLabel& buffer   = this->get<0>();
    ox::HLabel& exchange = this->get<1>();
    ox::HLabel& base     = this->get<2>();
    ox::HLabel& quote    = this->get<3>();

   public:
    Asset_name()
    {
        using namespace ox::pipe;
        buffer | fixed_width(1);
        exchange | fixed_width(10);
        base | fixed_width(6) | ox::Trait::Bold;
        quote | ox::Trait::Dim;
        *this | fixed_width(21);
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

class Quantity_edit : public ox::Textbox {
   public:
    sl::Signal<void(double)> quantity_updated;

   public:
    Quantity_edit()
    {
        *this | ox::pipe::fixed_height(1) | bg(crab::Almost_bg);
        this->disable_scrollwheel();
        this->initialize(0.);
    }

    void initialize(double value)
    {
        auto display = std::to_string(value);
        if (value == 0.)
            display = "0";
        else {
            format_decimal_zeros(display);
            insert_thousands_separators(display);
        }
        this->set_contents(display);
        quantity_updated.emit(value);
        value_ = value;
    }

    auto quantity() const -> double { return value_; }

   protected:
    auto key_press_event(ox::Key k) -> bool override
    {
        switch (k) {
            case ox::Key::Backspace:
            case ox::Key::Backspace_1:
            case ox::Key::Backspace_2:
            case ox::Key::Delete: {
                auto const result = Textbox::key_press_event(k);
                if (auto const dbl = get_double(this->contents().str()); dbl) {
                    value_ = *dbl;
                    quantity_updated.emit(value_);
                }
                return result;
            }
            case ox::Key::Arrow_right:
            case ox::Key::Arrow_left:
            case ox::Key::Arrow_up:
            case ox::Key::Arrow_down: return Textbox::key_press_event(k);
            default: break;
        }
        if (static_cast<std::underlying_type_t<ox::Key>>(k) > 127)
            return true;

        auto const ch = key_to_char(k);
        if (validate_char(ch)) {
            if (auto const dbl = get_double(this->generate_string(ch)); dbl) {
                value_ = *dbl;
                quantity_updated.emit(value_);
                return Textbox::key_press_event(k);
            }
        }
        return true;
    }

   private:
    /// Generate the string that will be made once the char is commited.
    /** Used for validation before sending the char to the Textbox base class */
    auto generate_string(char c) -> std::string
    {
        auto result = this->contents().str();
        result.insert(this->cursor_index(), 1, c);
        return result;
    }

   private:
    [[nodiscard]] static auto validate_char(char c) -> bool
    {
        return std::isdigit(c) || c == ',' || c == '.';
    }

    /// Returns parsed double if valid, std::nullopt if not valid.
    [[nodiscard]] static auto get_double(std::string input)
        -> std::optional<double>
    {
        input.erase(std::remove(std::begin(input), std::end(input), ','),
                    std::end(input));
        if (input.empty() || input == ".")
            return 0.;
        auto count = std::size_t{0};
        try {
            auto const result = std::stod(input, &count);
            if (count == input.size())
                return result;
            else
                return std::nullopt;
        }
        catch (std::exception const&) {
            return std::nullopt;
        }
    }

   private:
    double value_ = 0;
};

/// Settable as a string abbriviation, displayed in its symbolic representation.
class Currency_display : public ox::HLabel {
   public:
    Currency_display(std::string x = "")
    {
        using namespace ox::pipe;
        *this | fixed_width(3) | fixed_height(1) | align_center();
        if (!x.empty())
            this->set(std::move(x));
    }

   public:
    /// Set the currency to display, \p x is a string abbriviation.
    /** Uses an underscore if no symbol found for the given currency. */
    void set(std::string x)
    {
        currency_ = std::move(x);
        this->ox::HLabel::set_text(currency_to_symbol(currency_));
    }

    /// Return the string abbriviation currency was set with.
    [[nodiscard]] auto currency() const -> std::string const&
    {
        return currency_;
    }

   private:
    std::string currency_;
};

/// Displays an amount passed in as a string or double.
/** Inserts thousands separators and formats decimal zeros if needed. */
class Amount_display : public ox::HLabel {
   public:
    void set(double amount)
    {
        double_ = amount;
        string_ = std::to_string(amount);
        format_decimal_zeros(string_);
        insert_thousands_separators(string_);
        this->ox::HLabel::set_text(string_);
    }

    void set(std::string amount)
    {
        string_ = std::move(amount);
        double_ = std::stod(string_);
        format_decimal_zeros(string_);
        insert_thousands_separators(string_);
        this->ox::HLabel::set_text(string_);
    }

    /// Return set value as a double.
    [[nodiscard]] auto as_double() const -> double { return double_; }

    /// Return set value as it was passed in, without formatting.
    [[nodiscard]] auto as_string() const -> std::string const&
    {
        return string_;
    }

   private:
    double double_;
    std::string string_;
};

class Price_display : public ox::HPair<Currency_display, Amount_display> {
   public:
    Currency_display& currency = this->first;
    Amount_display& amount     = this->second;
};

/// Displays an amount passed in as a string or double, aligns on decimal place.
/** Inserts thousands separators and formats decimal zeros if needed, \p offset
 *  is used for decimal alignment, if \p hundredths_round is true, rounds. */
class Aligned_amount_display : public ox::HLabel {
   public:
    void set(double amount)
    {
        double_ = amount;
        string_ = std::to_string(amount);
        format_decimal_zeros(string_);
        insert_thousands_separators(string_);
        if (round_hundredths_)
            string_ = hundredths_round(string_);
        string_ = align_decimal(string_, offset_);
        this->ox::HLabel::set_text(string_);
    }

    void set(std::string amount)
    {
        string_ = std::move(amount);
        double_ = std::stod(string_);
        format_decimal_zeros(string_);
        insert_thousands_separators(string_);
        if (round_hundredths_)
            string_ = hundredths_round(string_);
        string_ = align_decimal(string_, offset_);
        this->ox::HLabel::set_text(string_);
    }

    void set_offset(std::size_t x)
    {
        offset_ = x;
        if (!string_.empty())
            this->set(string_);
    }

    void round_to_hundredths(bool x)
    {
        round_hundredths_ = x;
        if (!string_.empty())
            this->set(string_);
    }

    /// Return set value as a double.
    [[nodiscard]] auto as_double() const -> double { return double_; }

    /// Return set value as it was passed in, without formatting.
    [[nodiscard]] auto as_string() const -> std::string const&
    {
        return string_;
    }

   private:
    double double_;
    std::string string_;
    std::size_t offset_    = 0;
    bool round_hundredths_ = false;
};

/// Aligned price display and potential rounding to hundredths.
class Aligned_price_display
    : public ox::HPair<Currency_display, Aligned_amount_display> {
   public:
    Currency_display& currency     = this->first;
    Aligned_amount_display& amount = this->second;
};

class Percent_display : public ox::HArray<ox::HLabel, 2> {
   public:
    Percent_display()
    {
        using namespace ox::pipe;
        value | align_right();
        symbol | fixed_width(3) | align_center();
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
                                   ox::Widget,
                                   Quantity_edit,
                                   ox::Widget,
                                   Aligned_price_display,
                                   Div,
                                   Remove_btn> {
   public:
    Hamburger& hamburger            = this->get<0>();
    ox::Widget& buffer_1            = this->get<1>();
    Div& div1                       = this->get<2>();
    Asset_name& name                = this->get<3>();
    Indicator& indicator            = this->get<4>();
    Price_display& last_price       = this->get<5>();
    Percent_display& percent_change = this->get<6>();
    ox::Widget& buffer_2            = this->get<7>();
    Price_display& opening_price    = this->get<8>();
    ox::Widget& buffer_3            = this->get<9>();
    Quantity_edit& quantity         = this->get<10>();
    ox::Widget& buffer_4            = this->get<11>();
    Aligned_price_display& value    = this->get<12>();
    Div& div2                       = this->get<13>();
    Remove_btn& remove_btn          = this->get<14>();

   public:
    Listings()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);
        buffer_1 | fixed_width(1);
        last_price | fixed_width(14);
        buffer_2 | fixed_width(2);
        opening_price | fixed_width(16);
        buffer_3 | fixed_width(3);
        quantity | fixed_width(12);
        buffer_4 | fixed_width(2);
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
    Ticker(Asset asset, Stats stats, double quantity)
        : asset_{asset}, last_price_{stats.current_price}
    {
        listings.last_price.currency.set(asset.currency.quote);
        listings.opening_price.currency.set(asset.currency.quote);
        listings.value.currency.set(asset.currency.quote);
        if (is_USD_like(asset_.currency.quote)) {
            listings.value.amount.round_to_hundredths(true);
            listings.value.amount.set_offset(8);
        }
        listings.name.set(asset);

        listings.last_price.amount.set(stats.current_price);
        this->recalculate_percent_change();

        listings.quantity.initialize(quantity);
        listings.quantity.quantity_updated.connect([this](double quant) {
            this->update_value(quant, listings.last_price.amount.as_double());
        });

        this->update_opening_price(stats.opening_price);
    }

   public:
    void update_last_price(std::string const& value)
    {
        listings.last_price.amount.set(value);
        this->recalculate_percent_change();
        auto count       = std::size_t{0};
        auto const newer = std::stod(value, &count);
        assert(count == value.size());
        if (newer > last_price_)
            listings.indicator.emit_positive();
        else if (newer < last_price_)
            listings.indicator.emit_negative();
        this->update_value(listings.quantity.quantity(),
                           listings.last_price.amount.as_double());
        last_price_ = newer;
    }

    void update_opening_price(double value)
    {
        opening_price_ = value;
        listings.opening_price.amount.set(value);
        this->recalculate_percent_change();
    }

    [[nodiscard]] auto asset() const -> Asset const& { return asset_; }

    [[nodiscard]] auto quantity() const -> double
    {
        return listings.quantity.quantity();
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
        if (opening_price_ == 0.)
            return;
        listings.percent_change.set_percent(
            100. * ((last_price_ - opening_price_) / opening_price_));
    }

    void update_value(double quantity, double last_price)
    {
        listings.value.amount.set(calc_value(quantity, last_price));
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
    void add_ticker(Asset const& asset, double quantity)
    {
        if (this->find_ticker(asset) != nullptr)
            return;  // Already Added
        markets_.subscribe(asset);
        markets_.request_stats(asset);
        auto& child = this->make_child(asset, Stats{-1., 0.}, quantity);
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

class Column_labels : public ox::HArray<ox::HLabel, 11> {
   public:
    ox::HLabel& buffer_1       = this->get<0>();
    ox::HLabel& name           = this->get<1>();
    ox::HLabel& buffer_2       = this->get<2>();
    ox::HLabel& last_price     = this->get<3>();
    ox::HLabel& percent_change = this->get<4>();
    ox::HLabel& buffer_3       = this->get<5>();
    ox::HLabel& opening_price  = this->get<6>();
    ox::HLabel& buffer_4       = this->get<7>();
    ox::HLabel& quantity       = this->get<8>();
    ox::HLabel& buffer_5       = this->get<9>();
    ox::HLabel& value          = this->get<10>();

   public:
    Column_labels()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);
        buffer_1 | fixed_width(5);
        name.set_text(U" Asset" | ox::Trait::Bold);
        name | fixed_width(22);
        buffer_2 | fixed_width(2);
        last_price.set_text(U"   Last Price" | ox::Trait::Bold);
        last_price | fixed_width(14);
        percent_change.set_text(U"Change   " | ox::Trait::Bold);
        percent_change | align_right() | fixed_width(18);
        buffer_3 | fixed_width(2);
        opening_price.set_text(U"   Opening Price" | ox::Trait::Bold);
        opening_price | fixed_width(16);
        buffer_4 | fixed_width(3);
        quantity.set_text(U"Quantity" | ox::Trait::Bold);
        quantity | fixed_width(12);
        buffer_5 | fixed_width(2);
        value.set_text(U"   Value" | ox::Trait::Bold);
    }
};

}  // namespace crab
#endif  // CRAB_TICKER_LIST_HPP
