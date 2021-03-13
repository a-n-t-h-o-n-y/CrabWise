#ifndef CRAB_NET_TOTALS_HPP
#define CRAB_NET_TOTALS_HPP
#include <termox/termox.hpp>
#include <termox/widget/pipe.hpp>
#include <termox/widget/widget.hpp>

#include "line.hpp"
#include "palette.hpp"
#include "price_display.hpp"

namespace crab {

class Top_line : public ox::HTuple<ox::Widget, HLine> {
   public:
    ox::Widget& buffer = this->get<0>();
    HLine& line        = this->get<1>();

   public:
    Top_line()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);
        buffer | fixed_width(6);
    }
};

class Net_totals : public ox::HTuple<ox::HLabel,
                                     ox::Widget,
                                     Aligned_price_display,
                                     ox::Widget,
                                     Aligned_price_display,
                                     Aligned_price_display,
                                     ox::Widget> {
   public:
    ox::HLabel& title               = this->get<0>();
    ox::Widget& buffer_1            = this->get<1>();
    Aligned_price_display& value    = this->get<2>();
    ox::Widget& buffer_2            = this->get<3>();
    Aligned_price_display& open_pl  = this->get<4>();
    Aligned_price_display& daily_pl = this->get<5>();
    ox::Widget& buffer_end          = this->get<6>();

   public:
    Net_totals(std::string const& quote_currency)
    {
        using namespace ox::pipe;
        *this | fixed_height(1) | descendants() | bg(crab::Almost_bg);

        title.set_text(U" Totals" | ox::Trait::Bold);
        title | fixed_width(21);
        buffer_1 | fixed_width(56);

        value.amount.round_to_hundredths(true);
        value.amount.set_offset(8);
        value.currency.set(quote_currency);
        value | fixed_width(14);

        buffer_2 | fixed_width(14);

        open_pl.amount.round_to_hundredths(true);
        open_pl.amount.set_offset(8);
        open_pl.currency.set(quote_currency);
        open_pl | fixed_width(14);

        daily_pl.amount.round_to_hundredths(true);
        daily_pl.amount.set_offset(8);
        daily_pl.currency.set(quote_currency);
        daily_pl | fixed_width(14);
    }

   public:
    /// Return the set quote currency.
    auto quote() const -> std::string const&
    {
        return value.currency.currency();
    }
};

// Page one is blank.
class Net_totals_container : public ox::layout::Stack<Net_totals> {
   public:
    Net_totals_container()
    {
        this->make_page("");
        this->set_active_page(0);
        *this | ox::pipe::fixed_height(1);
    }

   public:
    /// Add a new child Widget associated with \p quote currency.
    auto append_net_totals(std::string const& quote) -> Net_totals&
    {
        auto& child = this->make_page(quote);
        if (this->child_count() == 2)
            this->set_active_page(1);
        return child;
    }

    void update_value(std::string const& quote, double sum)
    {
        auto at = this->find_child(quote);
        if (at != nullptr)
            at->value.amount.set(sum);
    }

    void update_open_pl(std::string const& quote, double sum)
    {
        auto at = this->find_child(quote);
        if (at != nullptr)
            at->value.amount.set(sum);
    }

    void update_daily_pl(std::string const& quote, double sum)
    {
        auto at = this->find_child(quote);
        if (at != nullptr)
            at->value.amount.set(sum);
    }

   protected:
    /// Return nullptr if not found, searches by quote currency.
    [[nodiscard]] auto find_child(std::string const& quote) -> Net_totals*
    {
        return this->find_child_if(
            [&quote](Net_totals& child) { return child.quote() == quote; });
    }
};

class Net_totals_manager : public Net_totals_container {
   public:
    void update_value(std::string const& quote, double sum)
    {
        auto* const at = this->find_child(quote);
        auto& child    = (at == nullptr) ? this->append_net_totals(quote) : *at;
        child.value.amount.set(sum);
    }

    void update_open_pl(std::string const& quote, double sum)
    {
        auto* const at = this->find_child(quote);
        auto& child    = (at == nullptr) ? this->append_net_totals(quote) : *at;
        child.open_pl.amount.set(sum);
    }

    void update_daily_pl(std::string const& quote, double sum)
    {
        auto* const at = this->find_child(quote);
        auto& child    = (at == nullptr) ? this->append_net_totals(quote) : *at;
        child.daily_pl.amount.set(sum);
    }

    void flip_forward()
    {
        auto const child_count = this->child_count();
        auto const active_page = this->active_page_index();
        if (child_count == 1)
            return;
        if ((active_page + 1) == child_count) {
            this->set_active_page(1);
            return;
        }
        this->set_active_page(active_page + 1);
    }

    void flip_backward()
    {
        auto const child_count = this->child_count();
        auto const active_page = this->active_page_index();
        if (child_count == 1)
            return;
        if (active_page == 1)
            this->set_active_page(child_count - 1);
        this->set_active_page(active_page - 1);
    }
};

class Flippers : public ox::HTuple<ox::Button, VLine, ox::Button> {
   public:
    ox::Button& bkwd = this->get<0>();
    VLine& div       = this->get<1>();
    ox::Button& fwd  = this->get<2>();

   public:
    Flippers()
    {
        using namespace ox::pipe;
        *this | fixed_width(7) | children() | bg(crab::Almost_bg);
        bkwd | fixed_width(3) | label(U"<" | ox::Trait::Bold);
        div | fixed_width(1);
        fwd | fixed_width(3) | label(U">" | ox::Trait::Bold);
    }
};

class All_net_totals : public ox::HPair<Flippers, Net_totals_manager> {
   public:
    Flippers& flippers                     = this->first;
    Net_totals_manager& net_totals_manager = this->second;

   public:
    All_net_totals()
    {
        *this | ox::pipe::fixed_height(1);
        flippers.fwd.pressed.connect(
            [this] { net_totals_manager.flip_forward(); });
        flippers.bkwd.pressed.connect(
            [this] { net_totals_manager.flip_backward(); });
    }
};

}  // namespace crab
#endif  // CRAB_NET_TOTALS_HPP
