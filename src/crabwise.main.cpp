#include <filesystem>
#include <iostream>
#include <string_view>

#include <termox/termox.hpp>

#include "crabwise.hpp"
#include "filenames.hpp"
#include "markets/error.hpp"

int main(int argc, char* argv[])
{
    try {
        auto const key_path = crab::finnhub_key_filepath();
        if (argc > 1) {
            auto key  = std::string_view{argv[1]};
            auto file = std::ofstream{key_path};
            file << key;
        }
        if (!std::filesystem::exists(key_path)) {
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
    return ox::System{ox::Mouse_mode::Drag}.run<crab::Crabwise>();
}
