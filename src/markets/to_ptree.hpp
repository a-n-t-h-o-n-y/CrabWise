#ifndef CRAB_MARKETS_NETWORK_TO_PTREE_HPP
#define CRAB_MARKETS_NETWORK_TO_PTREE_HPP
#include <sstream>
#include <string>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace crab {

/// Parses json text into boost::ptree
[[nodiscard]] inline auto to_ptree(std::string const& message)
    -> boost::property_tree::ptree
{
    auto tree = boost::property_tree::ptree{};
    auto ss   = std::stringstream{message};
    read_json(ss, tree);
    return tree;
}

}  // namespace crab
#endif  // CRAB_MARKETS_NETWORK_TO_PTREE_HPP
