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
    /// Set percent, assumes it is scaled to 100's already.
    void set_percent(double x)
    {
        value_ = x;
        value.set_text(round_and_to_string(x, 2));
    }

    /// Return the set percent, scalled to 100's
    [[nodiscard]] auto get_percent() const -> double { return value_; }

   public:
    ox::HLabel& value  = this->get<0>();
    ox::HLabel& symbol = this->get<1>();

   private:
    double value_ = 0.;
};

}  // namespace crab
#endif  // CRAB_PERCENT_DISPLAY_HPP
