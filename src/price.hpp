#ifndef CRAB_PRICE_HPP
#define CRAB_PRICE_HPP
#include <string>

#include "asset.hpp"

namespace crab {

struct Price {
    std::string value;
    Asset asset;
};

}  // namespace crab
#endif  // CRAB_PRICE_HPP
