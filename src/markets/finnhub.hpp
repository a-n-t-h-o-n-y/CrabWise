#ifndef CRAB_MARKETS_FINNHUB_HPP
#define CRAB_MARKETS_FINNHUB_HPP
#include <vector>

#include "../currency_pair.hpp"
#include "network/https_socket.hpp"
#include "network/websocket.hpp"

namespace crab {

// Just stocks at first... only a single connection, so crypto will work, but
// you'll want to propagate the market too, with the pair.
class Finnhub {
   public:
   private:
    Websocket ws_;
    mutable std::vector<Currency_pair> currency_pairs_;
    mutable HTTPS_socket https_socket_;
};

}  // namespace crab
#endif  // CRAB_MARKETS_FINNHUB_HPP
