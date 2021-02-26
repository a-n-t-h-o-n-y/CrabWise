#ifndef CRAB_MARKETS_TO_ISO8601_HPP
#define CRAB_MARKETS_TO_ISO8601_HPP
#include <ctime>
#include <string>

#include "../candle.hpp"

namespace crab {

[[nodiscard]] inline auto to_iso8601(crab::Candle::Time_point_t at)
    -> std::string
{
    auto time_t_at = crab::Candle::Clock_t::to_time_t(at);
    auto buf       = std::string(21, '\0');
    std::strftime(buf.data(), buf.size(), "%FT%TZ", gmtime(&time_t_at));
    return buf;
}

}  // namespace crab
#endif  // CRAB_MARKETS_TO_ISO8601_HPP
