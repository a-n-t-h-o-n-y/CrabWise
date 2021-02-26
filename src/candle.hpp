#ifndef CRAB_CANDLE_HPP
#define CRAB_CANDLE_HPP
#include <chrono>
#include <string>

#include "currency_pair.hpp"

namespace crab {

struct Candle {
   public:
    using Clock_t      = std::chrono::system_clock;
    using Time_point_t = Clock_t::time_point;
    using Duration_t   = std::chrono::seconds;

   public:
    Currency_pair currency;
    Time_point_t start_time;
    Duration_t duration;
    std::string low;
    std::string high;
    std::string opening;
    std::string closing;
    std::string volume;
};

}  // namespace crab
#endif  // CRAB_CANDLE_HPP
