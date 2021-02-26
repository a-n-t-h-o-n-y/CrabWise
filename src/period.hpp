#ifndef CRAB_PERIOD_HPP
#define CRAB_PERIOD_HPP
#include <chrono>
#include <utility>

#include "candle.hpp"

namespace crab {

/// Period of time, up to the present time.
enum class Period {
    Last_hour,
    Last_quarter_day,
    Last_half_day,
    Last_day,
    Last_week,
    Last_month,
    Last_year
};

/// Return interval from now - p  to now. first return value is start.
// [[nodiscard]] auto inline period_to_chrono(Period p)
//     -> std::pair<Candle::Time_point_t, Candle::Time_point_t>
// {
//     auto const now = Candle::Clock_t::now();
//     auto interval  = Candle::Duration_t{};
//     switch (p) {
//         case Period::Last_hour: interval = std::chrono::hours{1}; break;
//         case Period::Last_quarter_day: interval = std::chrono::hours{6};
//         break; case Period::Last_half_day: interval = std::chrono::hours{12};
//         break; case Period::Last_day: interval = std::chrono::hours{24};
//         break; case Period::Last_week: interval = std::chrono::hours{168};
//         break; case Period::Last_month: interval = std::chrono::hours{720};
//         break; case Period::Last_year: interval = std::chrono::hours{8'760};
//         break;
//     }
//     return {now - interval, now};
// }

}  // namespace crab
#endif  // CRAB_PERIOD_HPP
