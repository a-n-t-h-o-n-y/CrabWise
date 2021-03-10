#ifndef CRAB_SYMBOL_ID_JSON_HPP
#define CRAB_SYMBOL_ID_JSON_HPP
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "asset.hpp"

namespace crab {

/// Query and write out symbol ids to json file in ~/Documents/crabwise
void write_ids_json();

/// Read in symbol ids and cooresponding Assets from given \p filepath json.
auto read_ids_json(std::filesystem::path const& filepath)
    -> std::vector<std::pair<std::string, Asset>>;

}  // namespace crab
#endif  // CRAB_SYMBOL_ID_JSON_HPP