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
                this->ticker_list.add_ticker(asset, 0.);
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
        status_bar.save_btn.pressed.connect([this] { this->save_state(); });
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
                      std::to_string(ticker.quantity()) + '\n');
        return result;
    }

    [[nodiscard]] static auto generate_save_string(Ticker_list& list)
        -> std::string
    {
        auto result   = std::string{};
        auto exchange = std::string{};
        for (Ticker& child : list.get_children()) {
            result.append(to_string(child, exchange != child.asset().exchange));
            exchange = child.asset().exchange;
        }
        return result;
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
        for (auto const& [asset, quantity] : init_assets)
            app_space.ticker_list.add_ticker(asset, quantity);
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
        -> std::tuple<std::string, Currency_pair, double>
    {
        auto ss    = std::istringstream{line};
        auto first = std::string{};

        assert((bool)ss);
        ss >> first;

        if (first[first.size() - 1] == ':') {
            first.pop_back();
            return {upper(first), {"", ""}, 0.};
        }
        if (first.front() == '#')
            return {"", {"", ""}, 0.};

        auto quote = std::string{};
        ss >> quote;

        auto quantity = 0.;
        if (ss)
            ss >> quantity;
        return {"", {upper(first), upper(quote)}, quantity};
    }

    /// Return true if string is all space characters or is empty.
    [[nodiscard]] static auto all_is_space(std::string const& x) -> bool
    {
        for (char c : x)
            if (!std::isspace(c))
                return false;
        return true;
    }

    // Quantity is optional, defaults to zero
    // assets.txt Format
    // Exchange:
    //     Base Quote Quantity
    //     Base Quote Quantity
    //     Base Quote Quantity
    //
    // Stock:
    //     Symbol Quantity
    //     Symbol Quantity
    // # Comment
    [[nodiscard]] static auto parse_init_file(fs::path const& filepath)
        -> std::vector<std::pair<Asset, double>>
    {
        if (!fs::exists(filepath))
            return {};
        auto result           = std::vector<std::pair<Asset, double>>{};
        auto file             = std::ifstream{filepath.string()};
        auto line             = std::string{};
        auto current_exchange = std::string{};
        while (std::getline(file, line, '\n')) {
            if (all_is_space(line))
                continue;
            auto const [exchange, currency, quantity] = parse_line(line);
            if (exchange.empty() && currency.base.empty())
                continue;
            if (exchange.empty()) {
                if (current_exchange == "STOCK")
                    result.push_back({{"", {currency.base, "USD"}}, quantity});
                else
                    result.push_back({{current_exchange, currency}, quantity});
            }
            else
                current_exchange = exchange;
        }
        return result;
    }
};

}  // namespace crab
#endif  // CRAB_CRABWISE_HPP
