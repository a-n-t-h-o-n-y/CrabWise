#ifndef CRAB_ASSET_PICKER_HPP
#define CRAB_ASSET_PICKER_HPP
#include <string>

#include <termox/termox.hpp>

#include "asset.hpp"
#include "palette.hpp"
#include "search_result.hpp"

namespace crab {

class Search_input
    : public ox::HTuple<ox::Spinner_fall_three, ox::Line_edit, ox::Button> {
   public:
    ox::Spinner_fall_three& spinner = this->get<0>();
    ox::Line_edit& search_field     = this->get<1>();
    ox::Button& button              = this->get<2>();

   public:
    sl::Signal<void(std::string const&)> search_request;

   public:
    Search_input()
    {
        using namespace ox::pipe;

        *this | fixed_height(1);
        button | label(U" Search" | ox::Trait::Bold) | fixed_width(8) |
            bg(crab::Light_gray) | fg(crab::Background);
        spinner | bg(crab::Almost_bg);
        spinner.start();
        search_field | bg(crab::Almost_bg);

        search_field.edit_finished.connect(
            [this](std::string const& s) { this->search_request(s); });
        button.pressed.connect([this] {
            this->search_request(this->search_field.contents().str());
        });
    }
};

class Asset_btn : public ox::HThin_button {
   public:
    Asset_btn(Search_result const& search_result)
        : ox::HThin_button{to_display(search_result)}
    {}

   private:
    [[nodiscard]] static auto to_display(Search_result const& sr)
        -> ox::Glyph_string
    {
        if (sr.type == "Crypto")
            return sr.description;
        return (ox::Glyph_string{sr.asset.currency.base} | ox::Trait::Bold)
            .append(' ')
            .append(sr.description);
    }
};

class Result_subgroup_btns
    : public ox::Passive<ox::layout::Vertical<Asset_btn>> {
   private:
    using Base_t = ox::Passive<ox::layout::Vertical<Asset_btn>>;

   public:
    sl::Signal<void(Asset const&)> selected;

   public:
    void add_result(Search_result const& search_result)
    {
        this->make_child(search_result)
            .pressed.connect(
                [this, asset = search_result.asset] { this->selected(asset); });
    }

    void clear_results() { this->delete_all_children(); }
};

class Results_subgroup
    : public ox::Passive<
          ox::VPair<ox::HLabel,
                    ox::HPair<ox::VScrollbar, Result_subgroup_btns>>> {
   public:
    ox::HLabel& label          = this->first;
    ox::VScrollbar& scrollbar  = this->second.first;
    Result_subgroup_btns& btns = this->second.second;

   public:
    sl::Signal<void(Asset const&)>& selected = btns.selected;

   public:
    Results_subgroup()
    {
        link(scrollbar, btns);
        label | ox::Trait::Bold | ox::Trait::Underline |
            ox::pipe::align_center();
        btns.height_policy.policy_updated.connect(
            [this] { this->second.height_policy = btns.height_policy; });
    }

   public:
    void add_result(Search_result const& search_result)
    {
        btns.add_result(search_result);
    }

    void clear_results() { btns.clear_results(); }
};

class Search_results_groups
    : public ox::Passive<ox::VArray<Results_subgroup, 2>> {
   public:
    sl::Signal<void(Asset const&)> selected;

   public:
    Results_subgroup& crypto = this->get<0>();
    Results_subgroup& stocks = this->get<1>();

   public:
    Search_results_groups()
    {
        crypto.selected.connect(
            [this](Asset const& a) { this->selected.emit(a); });
        stocks.selected.connect(
            [this](Asset const& a) { this->selected.emit(a); });
        crypto.label.set_text(U"Crypto");
        crypto.label | fg(crab::Light_gray);
        stocks.label.set_text(U"Stocks");
        stocks.label | fg(crab::Light_gray);
    }

   public:
    void add_result(Search_result const& search_result)
    {
        if (search_result.type == "Crypto")
            crypto.add_result(search_result);
        else if (search_result.type == "Common Stock")
            stocks.add_result(search_result);
    }

    void clear_results()
    {
        crypto.clear_results();
        stocks.clear_results();
    }
};

class Search_results : public ox::VPair<Search_results_groups, ox::Widget> {
   public:
    Search_results_groups& groups = this->first;
    ox::Widget& buffer            = this->second;

   public:
    sl::Signal<void(Asset const&)>& selected = groups.selected;

   public:
    Search_results() { buffer.install_event_filter(groups.stocks.scrollbar); }

   public:
    void add_result(Search_result const& search_result)
    {
        groups.add_result(search_result);
    }

    void clear_results() { groups.clear_results(); }
};

class Info_box : public ox::VAccordion<ox::HPair<ox::VScrollbar, ox::Textbox>> {
   private:
    using Base_t = ox::VAccordion<ox::HPair<ox::VScrollbar, ox::Textbox>>;

   public:
    ox::VScrollbar& scrollbar = this->wrapped().first;
    ox::Textbox& textbox      = this->wrapped().second;

   public:
    Info_box() : Base_t{{U"About 🦀", ox::Align::Center}}
    {
        link(scrollbar, textbox);

        this->wrapped() | ox::pipe::fixed_height(12);

        textbox.disable_input();
        textbox | ox::pipe::no_focus();
        add_info(textbox);
    }

   private:
    static void add_info(ox::Textbox& tb)
    {
        using ox::Trait;
        auto const div = [](ox::Glyph_string& x) {
            auto constexpr width = 24;
            x.append(U'\n');
            x.append(std::u32string(width, U'─') | fg(crab::Red));
            x.append(U'\n');
        };

        auto info = ox::Glyph_string{U"finnhub.key" | Trait::Bold};
        info.append(U": *Required File* Get a free Finnhub key from ");
        info.append(U"finnhub.io" | Trait::Underline);
        info.append(
            U" and place it in the same directory the app will run in.");

        div(info);

        info.append(U"assets.txt" | Trait::Bold);
        info.append(
            U": *Optional File* A list of assets to initialize the program "
            U"with. Located in the same directory the app is running from.\n");
        info.append(U"Format:\n");
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Exchange");
        info.append(U']' | fg(crab::Gray));
        info.append(U":\n");
        info.append(U"  ");
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Base");
        info.append(U']' | fg(crab::Gray));
        info.append(U' ');
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Quote");
        info.append(U']' | fg(crab::Gray));
        info.append(U' ');
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Count");
        info.append(U']' | fg(crab::Gray));
        info.append(U'\n');
        info.append(U"  ");
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Base");
        info.append(U']' | fg(crab::Gray));
        info.append(U' ');
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Quote");
        info.append(U']' | fg(crab::Gray));
        info.append(U'\n');
        info.append(U"Stock:\n");
        info.append(U"  ");
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Symbol");
        info.append(U']' | fg(crab::Gray));
        info.append(U' ');
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Count");
        info.append(U']' | fg(crab::Gray));
        info.append(U'\n');
        info.append(U"  ");
        info.append(U'[' | fg(crab::Gray));
        info.append(U"Symbol");
        info.append(U']' | fg(crab::Gray));
        info.append(U'\n');
        info.append(U"# Comment");

        div(info);

        info.append(
            U"Handle 𝍢 can be clicked and dragged to rearrange ordering. "
            U"Reorder only happens when mouse moves from one handle to another "
            U"handle.");

        div(info);

        info.append(U"Press 'x' to remove an asset.");

        div(info);

        info.append(
            U"Search relies on the Finnhub API, best when you know exactly the "
            U"asset you are searching for. Include the exchange, and the base "
            U"and quote currencies, divided by a '/' for best results. Yeah, "
            U"search result scrolling is weird.");

        tb.clear();
        tb.set_contents(info);
    }
};

class Asset_picker
    : public ox::HAccordion<ox::VTuple<Search_input, Search_results, Info_box>,
                            ox::Bar_position::Last> {
   private:
    using Base_t =
        ox::HAccordion<ox::VTuple<Search_input, Search_results, Info_box>,
                       ox::Bar_position::Last>;

   public:
    Search_input& search_input     = this->wrapped().get<0>();
    Search_results& search_results = this->wrapped().get<1>();
    Info_box& info_box             = this->wrapped().get<2>();

   public:
    Asset_picker() : Base_t{{U"Stonk Selector", ox::Align::Center}}
    {
        this->wrapped() | ox::pipe::fixed_width(26);
    }
};

}  // namespace crab
#endif  // CRAB_ASSET_PICKER_HPP
