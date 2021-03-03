#ifndef CRAB_STATS_HPP
#define CRAB_STATS_HPP
#include <string>

namespace crab {

/// Stats about an Asset.
struct Stats {
    std::string current_price;
    std::string opening_price;
};

}  // namespace crab
#endif  // CRAB_STATS_HPP
