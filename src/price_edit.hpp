#ifndef CRAB_PRICE_EDIT_HPP
#define CRAB_PRICE_EDIT_HPP
#include <termox/termox.hpp>

#include "currency_display.hpp"
#include "quantity_edit.hpp"

namespace crab {

struct Price_edit : ox::HPair<Currency_display, Quantity_edit> {
    Currency_display& currency = this->first;
    Quantity_edit& amount      = this->second;
};

}  // namespace crab
#endif  // CRAB_PRICE_EDIT_HPP
