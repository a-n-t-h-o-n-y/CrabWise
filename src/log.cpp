#include "log.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>

#include "filenames.hpp"

namespace {

[[nodiscard]] auto timestamp() -> std::string
{
    using Clock_t     = std::chrono::system_clock;
    auto const now    = Clock_t::to_time_t(Clock_t::now());
    auto const now_tm = *std::localtime(&now);
    auto ss           = std::stringstream{};
    ss << std::put_time(&now_tm, "%F_%T");
    return ss.str();
}

[[nodiscard]] auto file() -> std::ofstream&
{
    static auto file_ = [] {
        auto fs = std::ofstream{crab::log_filepath(), std::ios_base::app};
        fs << '\n' << timestamp() << "--------------------------------\n";
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
