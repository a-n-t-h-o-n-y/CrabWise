#ifndef CRAB_FILENAMES_HPP
#define CRAB_FILENAMES_HPP
#include <cstdlib>
#include <filesystem>
#include <string>

#include "markets/error.hpp"

namespace crab {

/// Returns the current systems HOME env-var as a filesystem::path.
/** Throws Crab_error if HOME does not exist. */
inline auto home_directory() -> std::filesystem::path
{
    static auto const home = std::filesystem::path{[] {
        char const* result = std::getenv("HOME");
        if (result == nullptr)
            throw Crab_error{"Can't find HOME environment variable"};
        return result;
    }()};
    return home;
}

/// Return the ${HOME}/Documents directory path.
/** Throws Crab_error if it does not already exist. */
inline auto documents_directory() -> std::filesystem::path
{
    static auto const documents = home_directory() / "Documents";
    if (!std::filesystem::exists(documents))
        throw Crab_error{"Can't find ${HOME}/Documents/ directory"};
    return documents;
}

/// Returns the path to ~/Documents/crabwise.
/** Makes the directory if it does not exist. */
inline auto crabwise_data_directory() -> std::filesystem::path
{
    static auto const data = documents_directory() / "crabwise";
    if (!std::filesystem::exists(data)) {
        try {
            if (!std::filesystem::create_directory(data)) {
                throw Crab_error{
                    "Count not create ~/Documents/crabwise directory."};
            }
        }
        catch (std::filesystem::filesystem_error const& e) {
            throw Crab_error{
                "Error when creating ~/Documents/crabwise directory: " +
                std::string{e.what()}};
        }
    }
    return data;
}

/// Return path to finnhub.key file, file might not exist yet.
inline auto finnhub_key_filepath() -> std::filesystem::path
{
    return crabwise_data_directory() / "finnhub.key";
}

/// Return path to assets.txt file, file might not exist yet.
inline auto assets_filepath() -> std::filesystem::path
{
    return crabwise_data_directory() / "assets.txt";
}

/// Return path to crabwise.log file, file might not exist yet.
inline auto log_filepath() -> std::filesystem::path
{
    return crabwise_data_directory() / "crabwise.log";
}

}  // namespace crab
#endif  // CRAB_FILENAMES_HPP
