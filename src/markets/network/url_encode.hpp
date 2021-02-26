#ifndef CRAB_MARKETS_NETWORK_URL_ENCODE_HPP
#define CRAB_MARKETS_NETWORK_URL_ENCODE_HPP
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

namespace crab {

/// URL Encode the given string.
[[nodiscard]] inline auto url_encode(std::string const& x) -> std::string
{
    auto escaped = std::ostringstream{};
    escaped.fill('0');
    escaped << std::hex;
    for (char c : x) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            escaped << c;
        else {
            escaped << std::uppercase;
            escaped << '%' << std::setw(2) << int((unsigned char)c);
            escaped << std::nouppercase;
        }
    }
    return escaped.str();
}

}  // namespace crab
#endif  // CRAB_MARKETS_NETWORK_URL_ENCODE_HPP
