#ifndef CRAB_CRABWISE_HPP
#define CRAB_CRABWISE_HPP
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <termox/termox.hpp>

#include "asset_picker.hpp"
#include "filenames.hpp"
#include "filesystem.hpp"
#include "net_totals.hpp"
#include "palette.hpp"
#include "search_result.hpp"
#include "ticker_list.hpp"

namespace crab {

class Status_box : public ox::HTuple<ox::Widget, ox::HLabel> {
   private:
    ox::Widget& arrow     = this->get<0>();
    ox::HLabel& text_area = this->get<1>();

   public:
    Status_box()
    {
        *this | ox::pipe::fixed_height(1) | ox::pipe::descendants() |
            bg(crab::Foreground) | fg(crab::Background);
        arrow | ox::pipe::wallpaper(U'/') | ox::pipe::fixed_width(1) |
            bg(ox::Color::Foreground) | fg(ox::Color::Background);
        text_area | bg(ox::Color::Foreground) | fg(ox::Color::Background);
    }

   public:
    void set_status(ox::Glyph_string const& x) { text_area.set_text(x); }
};

class Status_bar : public ox::HPair<Status_box, ox::Button> {
   public:
    Status_box& status_box = this->first;
    ox::Button& save_btn   = this->second;

   public:
    Status_bar()
    {
        *this | ox::pipe::fixed_height(1);
        save_btn.set_label(U"Save" | ox::Trait::Bold);
        save_btn | bg(crab::Yellow) | fg(crab::Background) |
            ox::pipe::fixed_width(8);
    }

   public:
    void set_status(ox::Glyph_string const& x) { status_box.set_status(x); }
};

class App_space
    : public ox::HTuple<
          Asset_picker,
          ox::VTuple<ox::Widget,
                     Column_labels,
                     ox::HTuple<ox::VTuple<HLine, Ticker_list, ox::Widget>,
                                ox::VScrollbar>,
                     All_net_totals,
                     Status_bar>> {
   public:
    Asset_picker& asset_picker = this->get<0>();
    Column_labels& labels      = this->get<1>().get<1>();
    ox::Widget& top_line       = this->get<1>().get<0>();
    Ticker_list& ticker_list   = this->get<1>().get<2>().get<0>().get<1>();
    ox::Widget& bottom_buffer  = this->get<1>().get<2>().get<0>().get<2>();
    All_net_totals& net_totals = this->get<1>().get<3>();
    Status_bar& status_bar     = this->get<1>().get<4>();
    ox::VScrollbar& scrollbar  = this->get<1>().get<2>().get<1>();

   public:
    App_space()
    {
        link(scrollbar, ticker_list);
        bottom_buffer.install_event_filter(scrollbar);

        top_line | ox::pipe::fixed_height(1);

        asset_picker.search_results.selected.connect(
            [this](Asset const& asset) {
                this->ticker_list.add_ticker(asset, 0., 0.);
            });

        asset_picker.search_input.search_request.connect(
            [this](std::string const& s) {
                asset_picker.search_results.clear_results();
                this->ticker_list.request_search(s);
            });

        ticker_list.search_results_received.connect(
            [this](std::vector<Search_result> const& results) {
                asset_picker.search_results.clear_results();
                asset_picker.search_input.stop_spinner();
                for (auto const& search_result : results)
                    asset_picker.search_results.add_result(search_result);
            });
        ticker_list.value_total_updated.connect(
            [this](std::string const& quote, double sum) {
                net_totals.net_totals_manager.update_value(quote, sum);
            });
        ticker_list.open_pl_total_updated.connect(
            [this](std::string const& quote, double sum) {
                net_totals.net_totals_manager.update_open_pl(quote, sum);
            });
        ticker_list.daily_pl_total_updated.connect(
            [this](std::string const& quote, double sum) {
                net_totals.net_totals_manager.update_daily_pl(quote, sum);
            });

        status_bar.save_btn.pressed.connect([this] { this->save_state(); });

        setup_column_sorting(*this);
    }

   private:
    void save_state()
    {
        auto const filepath = assets_filepath();
        {
            auto file = std::ofstream{filepath.string()};
            file << generate_save_string(ticker_list);
        }
        status_bar.set_status("Snapshot saved to: " + filepath.string());
    }

   private:
    [[nodiscard]] static auto to_string(Ticker& ticker, bool add_exchange)
        -> std::string
    {
        auto result = std::string{};
        if (add_exchange) {
            auto exchange = ticker.asset().exchange;
            if (exchange.empty())
                exchange = "Stock";
            result.append(exchange + ":\n");
        }
        result.append("    " + ticker.asset().currency.base + ' ' +
                      ticker.asset().currency.quote + ' ' +
                      std::to_string(ticker.quantity()) + ' ' +
                      std::to_string(ticker.cost_basis()) + '\n');
        return result;
    }

    [[nodiscard]] static auto generate_save_string(Ticker_list& list)
        -> std::string
    {
        auto result = std::string{
            "# ~ CrabWise ~\n# Exchange:\n#   Base Quote [Quantity] "
            "[Cost Basis]\n\n"};
        auto exchange = std::string{};
        for (Ticker& child : list.get_children()) {
            result.append(to_string(child, exchange != child.asset().exchange));
            exchange = child.asset().exchange;
        }
        return result;
    }

    static void setup_column_sorting(App_space& w)
    {
        using namespace ox::pipe;
        auto constexpr to_lower = [](std::string const& x) -> std::string {
            auto result = std::string{};
            for (unsigned char c : x)
                result.push_back(std::tolower(c));
            return result;
        };
        auto const hook_up = [&w](auto& label_widget, auto const& asc_comp,
                                  auto const& desc_comp) {
            label_widget |
                on_mouse_press([&w, &asc_comp, &desc_comp,
                                ascending = false](auto const& m) mutable {
                    if (m.button == ox::Mouse::Button::Left) {
                        if (ascending)
                            w.ticker_list.sort(asc_comp);
                        else
                            w.ticker_list.sort(desc_comp);
                        ascending = !ascending;
                    }
                });
        };

        // Value
        auto value_acomp = [](auto const& a, auto const& b) {
            return a.listings.value.amount.as_double() <
                   b.listings.value.amount.as_double();
        };
        auto value_dcomp = [](auto const& a, auto const& b) {
            return a.listings.value.amount.as_double() >
                   b.listings.value.amount.as_double();
        };
        hook_up(w.labels.value, value_acomp, value_dcomp);

        // Open P&L
        auto open_pl_acomp = [](auto const& a, auto const& b) {
            return a.listings.open_pl.amount.as_double() <
                   b.listings.open_pl.amount.as_double();
        };
        auto open_pl_dcomp = [](auto const& a, auto const& b) {
            return a.listings.open_pl.amount.as_double() >
                   b.listings.open_pl.amount.as_double();
        };
        hook_up(w.labels.open_pl, open_pl_acomp, open_pl_dcomp);

        // Daily P&L
        auto daily_pl_acomp = [](auto const& a, auto const& b) {
            return a.listings.daily_pl.amount.as_double() <
                   b.listings.daily_pl.amount.as_double();
        };
        auto daily_pl_dcomp = [](auto const& a, auto const& b) {
            return a.listings.daily_pl.amount.as_double() >
                   b.listings.daily_pl.amount.as_double();
        };
        hook_up(w.labels.daily_pl, daily_pl_acomp, daily_pl_dcomp);

        // % Change
        auto percent_change_acomp = [](auto const& a, auto const& b) {
            return a.listings.percent_change.get_percent() <
                   b.listings.percent_change.get_percent();
        };
        auto percent_change_dcomp = [](auto const& a, auto const& b) {
            return a.listings.percent_change.get_percent() >
                   b.listings.percent_change.get_percent();
        };
        hook_up(w.labels.percent_change, percent_change_acomp,
                percent_change_dcomp);

        // Base
        auto base_acomp = [](auto const& a, auto const& b) {
            return a.listings.name.base.get_text().str() >
                   b.listings.name.base.get_text().str();
        };
        auto base_dcomp = [](auto const& a, auto const& b) {
            return a.listings.name.base.get_text().str() <
                   b.listings.name.base.get_text().str();
        };
        hook_up(w.labels.base, base_acomp, base_dcomp);

        // Market
        auto market_acomp = [to_lower](auto const& a, auto const& b) {
            return to_lower(a.listings.name.market.get_text().str()) >
                   to_lower(b.listings.name.market.get_text().str());
        };
        auto market_dcomp = [to_lower](auto const& a, auto const& b) {
            return to_lower(a.listings.name.market.get_text().str()) <
                   to_lower(b.listings.name.market.get_text().str());
        };
        hook_up(w.labels.market, market_acomp, market_dcomp);

        // Quote
        auto quote_acomp = [to_lower](auto const& a, auto const& b) {
            return to_lower(a.listings.name.quote.get_text().str()) >
                   to_lower(b.listings.name.quote.get_text().str());
        };
        auto quote_dcomp = [to_lower](auto const& a, auto const& b) {
            return to_lower(a.listings.name.quote.get_text().str()) <
                   to_lower(b.listings.name.quote.get_text().str());
        };
        hook_up(w.labels.quote, quote_acomp, quote_dcomp);
    }
};

class Crabwise : public ox::VTuple<ox::Titlebar, App_space> {
   public:
    ox::Titlebar& titlebar = this->get<0>();
    App_space& app_space   = this->get<1>();

   public:
    /// Reads \p assets.txt and adds currencies.
    Crabwise()
    {
        titlebar.title.set_text(U"CrabWise" | ox::Trait::Bold);
        ox::Terminal::set_palette(crab::palette);
        auto const init_assets = parse_init_file(assets_filepath());
        for (auto const& [asset, quantity, cost_basis] : init_assets)
            app_space.ticker_list.add_ticker(asset, quantity, cost_basis);
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
        -> std::tuple<std::string, Currency_pair, double, double>
    {
        auto ss    = std::istringstream{line};
        auto first = std::string{};

        assert((bool)ss);
        ss >> first;

        if (first[first.size() - 1] == ':') {
            first.pop_back();
            return {upper(first), {"", ""}, 0., 0.};
        }
        if (first.front() == '#')
            return {"", {"", ""}, 0., 0.};

        auto quote = std::string{};
        ss >> quote;

        auto quantity = 0.;
        if (ss)
            ss >> quantity;
        auto cost_basis = 0.;
        if (ss)
            ss >> cost_basis;
        return {"", {upper(first), upper(quote)}, quantity, cost_basis};
    }

    /// Return true if string is all space characters or is empty.
    [[nodiscard]] static auto all_is_space(std::string const& x) -> bool
    {
        for (char c : x)
            if (!std::isspace(c))
                return false;
        return true;
    }

    // Quantity and Cost_basis are optional, defaults to zero
    // assets.txt Format
    // Exchange:
    //     Base Quote Quantity Cost_basis
    //     Base Quote Quantity Cost_basis
    //     Base Quote Quantity Cost_basis
    //
    // Stock:
    //     Symbol Quantity Cost_basis
    //     Symbol Quantity Cost_basis
    // # Comment
    [[nodiscard]] static auto parse_init_file(fs::path const& filepath)
        -> std::vector<std::tuple<Asset, double, double>>
    {
        if (!fs::exists(filepath))
            return {};
        auto result = std::vector<std::tuple<Asset, double, double>>{};
        auto file   = std::ifstream{filepath.string()};
        auto line   = std::string{};
        auto current_exchange = std::string{};
        while (std::getline(file, line, '\n')) {
            if (all_is_space(line))
                continue;
            auto const [exchange, currency, quantity, cost_basis] =
                parse_line(line);
            if (exchange.empty() && currency.base.empty())
                continue;
            if (exchange.empty()) {
                if (current_exchange == "STOCK") {
                    result.push_back(
                        {{"", {currency.base, "USD"}}, quantity, cost_basis});
                }
                else {
                    result.push_back(
                        {{current_exchange, currency}, quantity, cost_basis});
                }
            }
            else
                current_exchange = exchange;
        }
        return result;
    }
};

}  // namespace crab
#endif  // CRAB_CRABWISE_HPP
