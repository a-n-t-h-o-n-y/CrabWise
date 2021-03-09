#include "log.hpp"

#include <fstream>
#include <mutex>
#include <string>

#include "filenames.hpp"

namespace {

[[nodiscard]] auto file() -> std::ofstream&
{
    static auto file_ = [] {
        auto fs = std::ofstream{crab::log_filepath(), std::ios_base::app};
        fs << "\n\n--------\n\n";
        return fs;
    }();
    return file_;
}

[[nodiscard]] auto file_mtx() -> std::mutex&
{
    static auto mtx_ = std::mutex{};
    return mtx_;
}

}  // namespace

namespace crab {

void log_status(std::string const& x)
{
    auto const lock = std::lock_guard{file_mtx()};
    file() << "Status: " << x << '\n' << std::flush;
}

void log_error(std::string const& x)
{
    auto const lock = std::lock_guard{file_mtx()};
    file() << "Error: " << x << '\n' << std::flush;
}

}  // namespace crab
