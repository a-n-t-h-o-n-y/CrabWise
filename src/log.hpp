#ifndef CRAB_SET_STATUS_HPP
#define CRAB_SET_STATUS_HPP
#include <string>

namespace crab {

/// log a status message to crabwise.log file.
void log_status(std::string const& x);

/// log an error message to crabwise.log file.
void log_error(std::string const& x);

}  // namespace crab
#endif  // CRAB_SET_STATUS_HPP
