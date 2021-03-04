#ifndef CRAB_ASSET_PICKER_HPP
#define CRAB_ASSET_PICKER_HPP
#include <string>

#include <termox/termox.hpp>

#include "asset.hpp"
#include "palette.hpp"
#include "search_result.hpp"
#include "termox/widget/pipe.hpp"
#include "termox/widget/widgets/scrollbar.hpp"
#include "termox/widget/widgets/spinner.hpp"

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
        spinner | bg(crab::Gray);
        spinner.start();
        search_field | bg(crab::Gray);

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
        stocks.label.set_text(U"Stocks");
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

class Asset_picker
    : public ox::HAccordion<ox::VPair<Search_input, Search_results>,
                            ox::Bar_position::Last> {
   private:
    using Base_t = ox::HAccordion<ox::VPair<Search_input, Search_results>,
                                  ox::Bar_position::Last>;

   public:
    Search_input& search_input     = this->wrapped().first;
    Search_results& search_results = this->wrapped().second;

   public:
    Asset_picker() : Base_t{{U"Stonk Selector", ox::Align::Center}}
    {
        this->wrapped() | ox::pipe::fixed_width(22);
    }
};

}  // namespace crab
#endif  // CRAB_ASSET_PICKER_HPP
