#ifndef CRAB_FORMAT_MONEY_HPP
#define CRAB_FORMAT_MONEY_HPP
#include <cassert>
#include <string>

namespace crab {

inline void insert_thousands_separators(std::string& value)
{
    auto decimal = value.rfind('.');
    assert(decimal != std::string::npos);
    while ((decimal - 3 < decimal) && (decimal - 3 != 0)) {
        decimal -= 3;
        value.insert(decimal, 1, ',');
    }
}

inline void format_decimal_places(std::string& value)
{
    auto const decimal = value.rfind('.');
    if (decimal == std::string::npos) {
        value.append(".00");
        return;
    }

    // Add Trailing Zeros
    while ((value.size() - decimal) < 3)
        value.push_back('0');

    // Remove Trailing Zeros
    while (value.size() - decimal > 3 && value.back() == '0')
        value.pop_back();
}

inline void format_money(std::string& money)
{
    // at least two decimal places, longer is fine.
    format_decimal_places(money);
    insert_thousands_separators(money);
}

}  // namespace crab
#endif  // CRAB_FORMAT_MONEY_HPP
