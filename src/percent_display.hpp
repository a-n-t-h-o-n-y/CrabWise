#ifndef CRAB_PERCENT_DISPLAY_HPP
#define CRAB_PERCENT_DISPLAY_HPP
#include <termox/termox.hpp>

#include "format_money.hpp"

namespace crab {

class Percent_display : public ox::HArray<ox::HLabel, 2> {
   public:
    Percent_display()
    {
        using namespace ox::pipe;
        value | align_right();
        symbol | fixed_width(3) | align_center();
        symbol.set_text(U"%");
    }

   public:
    void set_percent(double x) { value.set_text(round_and_to_string(x, 2)); }

   public:
    ox::HLabel& value  = this->get<0>();
    ox::HLabel& symbol = this->get<1>();
};

}  // namespace crab
#endif  // CRAB_PERCENT_DISPLAY_HPP
