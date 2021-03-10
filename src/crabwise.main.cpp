#include <chrono>
#include <iostream>
#include <string_view>
#include <thread>

#include <termox/termox.hpp>

#include "crabwise.hpp"
#include "filenames.hpp"
#include "filesystem.hpp"
#include "markets/error.hpp"
#include "symbol_id_json.hpp"

int main(int argc, char* argv[])
{
    // Write key to file if does not exist
    try {
        auto const key_path = crab::finnhub_key_filepath();
        if (argc > 1) {
            auto key  = std::string_view{argv[1]};
            auto file = std::ofstream{key_path.string()};
            file << key;
        }
        if (!crab::fs::exists(key_path)) {
            std::cerr << "Must pass in a Finnhub API Key on first use.\n"
                         "Go to finnhub.io and register for a free API key.\n"
                         "Then pass the key to the command line when opening "
                         "CrabWise.\n";
            return 1;
        }
    }
    catch (crab::Crab_error const& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    // Generate Symbol Ids from Finnhub, if they do not exist.
    if (!crab::fs::exists(crab::symbol_ids_json_filepath())) {
        try {
            std::cout << "Setting up Symbol ID database on first run.\n"
                      << "Will take a minute...\n";
            crab::write_ids_json();
            std::cout << "Symbol IDs initialized!\n";
            std::this_thread::sleep_for(std::chrono::milliseconds{400});
        }
        catch (std::exception const& e) {
            std::cerr << "Failed to generate symbol ids: " << e.what() << '\n';
        }
    }

    return ox::System{ox::Mouse_mode::Drag}.run<crab::Crabwise>();
}
