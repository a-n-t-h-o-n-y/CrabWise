#ifndef CRAB_MARKETS_ERROR_HPP
#define CRAB_MARKETS_ERROR_HPP
#include <stdexcept>

namespace crab {

class Crab_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

}  // namespace crab
#endif  // CRAB_MARKETS_ERROR_HPP
