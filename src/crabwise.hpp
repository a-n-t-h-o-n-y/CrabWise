#ifndef CRAB_CRABWISE_HPP
#define CRAB_CRABWISE_HPP
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <termox/termox.hpp>

#include "asset_picker.hpp"
#include "palette.hpp"
#include "search_result.hpp"
#include "ticker_list.hpp"

namespace crab {

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
    [[nodiscard]] static auto parse_init_file(std::string const& filename)
        -> std::vector<std::pair<Asset, double>>
    {
        auto result           = std::vector<std::pair<Asset, double>>{};
        auto file             = std::ifstream{filename};
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
