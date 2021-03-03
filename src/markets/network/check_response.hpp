#ifndef CRAB_MARKETS_NETWORK_CHECK_RESPONSE_HPP
#define CRAB_MARKETS_NETWORK_CHECK_RESPONSE_HPP
#include <string>
#include <utility>

#include "../error.hpp"
#include "https_socket.hpp"

namespace crab {

/// Throws error if code != 200, includes \p about in thrown error.what().
void inline check_response(HTTPS_socket::Response const& message,
                           std::string const& about)
{
    if (message.code == 200)
        return;
    auto what = "code: " + std::to_string(message.code) + '\n' + about + ": " +
                message.body;
    throw Crab_error{std::move(what)};
}

}  // namespace crab
#endif  // CRAB_MARKETS_NETWORK_CHECK_RESPONSE_HPP
