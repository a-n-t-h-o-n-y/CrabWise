#ifndef CRAB_FILENAMES_HPP
#define CRAB_FILENAMES_HPP
#include <cstdlib>
#include <string>

#include "filesystem.hpp"
#include "markets/error.hpp"

namespace crab {

/// Returns the current systems HOME env-var as a filesystem::path.
/** Throws Crab_error if HOME does not exist. */
[[nodiscard]] inline auto home_directory() -> fs::path
{
    static auto const home = fs::path{[] {
        char const* result = std::getenv("HOME");
        if (result == nullptr)
            throw Crab_error{"Can't find HOME environment variable"};
        return result;
    }()};
    return home;
}

/// Return the ${HOME}/Documents directory path.
/** Throws Crab_error if it does not already exist. */
[[nodiscard]] inline auto documents_directory() -> fs::path
{
    static auto const documents = home_directory() / "Documents";
    if (!fs::exists(documents))
        throw Crab_error{"Can't find ${HOME}/Documents/ directory"};
    return documents;
}

/// Returns the path to ~/Documents/crabwise.
/** Makes the directory if it does not exist. */
[[nodiscard]] inline auto crabwise_data_directory() -> fs::path
{
    static auto const data = documents_directory() / "crabwise";
    if (!fs::exists(data)) {
        try {
            if (!fs::create_directory(data)) {
                throw Crab_error{
                    "Count not create ~/Documents/crabwise directory."};
            }
        }
        catch (fs::filesystem_error const& e) {
            throw Crab_error{
                "Error when creating ~/Documents/crabwise directory: " +
                std::string{e.what()}};
        }
    }
    return data;
}

/// Return path to finnhub.key file, file might not exist yet.
[[nodiscard]] inline auto finnhub_key_filepath() -> fs::path
{
    return crabwise_data_directory() / "finnhub.key";
}

/// Return path to assets.txt file, file might not exist yet.
[[nodiscard]] inline auto assets_filepath() -> fs::path
{
    return crabwise_data_directory() / "assets.txt";
}

/// Return path to crabwise.log file, file might not exist yet.
[[nodiscard]] inline auto log_filepath() -> fs::path
{
    return crabwise_data_directory() / "crabwise.log";
}

/// Return path to ids.json file, file might not exist yet.
[[nodiscard]] inline auto symbol_ids_json_filepath() -> fs::path
{
    return crabwise_data_directory() / "ids.json";
}

}  // namespace crab
#endif  // CRAB_FILENAMES_HPP
