#ifndef CRAB_SEARCH_RESULT_HPP
#define CRAB_SEARCH_RESULT_HPP
#include <string>

#include "asset.hpp"

namespace crab {

struct Search_result {
    std::string type;
    std::string description;
    Asset asset;
};

}  // namespace crab
#endif  // CRAB_SEARCH_RESULT_HPP
