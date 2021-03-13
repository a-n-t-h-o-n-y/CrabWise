#ifndef CRAB_PRICE_DISPLAY_HPP
#define CRAB_PRICE_DISPLAY_HPP
#include "amount_display.hpp"
#include "currency_display.hpp"

namespace crab {

struct Price_display : ox::HPair<Currency_display, Amount_display> {
    Currency_display& currency = this->first;
    Amount_display& amount     = this->second;
};

/// Aligned price display and potential rounding to hundredths.
class Aligned_price_display
    : public ox::HPair<Currency_display, Aligned_amount_display> {
   public:
    Currency_display& currency     = this->first;
    Aligned_amount_display& amount = this->second;
};

}  // namespace crab
#endif  // CRAB_PRICE_DISPLAY_HPP
