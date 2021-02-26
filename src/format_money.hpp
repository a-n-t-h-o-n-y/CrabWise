#ifndef CRAB_FORMAT_MONEY_HPP
#define CRAB_FORMAT_MONEY_HPP
#include <string>

namespace crab {

inline void insert_thousands_separators(std::string& /*value*/)
{
    // TODO
    // search from end for '.' if not found, then initial is the last value,
    // otherwise its one past '.' from the reverse.
    // loop over 3 indices at a time, careful about offset after insertio
}

inline void format_money(std::string& money)
{
    // at least two decimal places, longer is fine.
    insert_thousands_separators(money);
}

}  // namespace crab
#endif  // CRAB_FORMAT_MONEY_HPP
