#ifndef CRAB_PRICE_HPP
#define CRAB_PRICE_HPP
#include <string>

#include "currency_pair.hpp"

namespace crab {

struct Price {
    std::string value;
    Currency_pair currency;
};

}  // namespace crab
#endif  // CRAB_PRICE_HPP
