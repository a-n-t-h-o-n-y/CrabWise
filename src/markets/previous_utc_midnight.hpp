#ifndef CRAB_MARKETS_PREVIOUS_UTC_MIDNIGHT_HPP
#define CRAB_MARKETS_PREVIOUS_UTC_MIDNIGHT_HPP
#include <chrono>
#include <ctime>

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/time_clock.hpp>

namespace crab {

/// Return the last UTC Midnight from now() as iso8601 string.
[[nodiscard]] inline auto previous_utc_midnight() -> std::string
{
    return to_iso_extended_string(
               boost::gregorian::day_clock::universal_day()) +
           "T00:00:00Z";
}

}  // namespace crab
#endif  // CRAB_MARKETS_PREVIOUS_UTC_MIDNIGHT_HPP
