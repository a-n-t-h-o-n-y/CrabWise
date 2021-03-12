#ifndef CRAB_FORMAT_MONEY_HPP
#define CRAB_FORMAT_MONEY_HPP
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

namespace crab {

/// Insert comma every three digits before the decimal.
inline void insert_thousands_separators(std::string& value)
{
    auto decimal = value.rfind('.');
    assert(decimal != std::string::npos);
    while ((decimal - 3 < decimal) && (decimal - 3 != 0)) {
        decimal -= 3;
        value.insert(decimal, 1, ',');
    }
}

/// Decimal zero formatting, appends zeros to ensure .00, removes trailing zeros
inline void format_decimal_zeros(std::string& value)
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

/// Round double and output as string.
[[nodiscard]] inline auto round_and_to_string(double value, int decimal_places)
    -> std::string
{
    auto ss = std::stringstream{};
    ss << std::fixed << std::setprecision(decimal_places) << value;
    return ss.str();
}

[[nodiscard]] inline auto currency_to_symbol(std::string const& x)
    -> std::string
{
    if (x == "USD")
        return "$";
    if (x == "ETH")
        return "Ξ";
    if (x == "BTC")
        return "₿";
    if (x == "XBT")
        return "₿";
    if (x == "EUR")
        return "€";
    if (x == "GBP")
        return "£";
    if (x == "DAI")
        return "◈";
    if (x == "USDC")
        return "ᐥC";
    if (x == "USDT")
        return "₮";
    if (x == "XRP")
        return "✕";
    if (x == "BCH")
        return "Ƀ";
    if (x == "BSV")
        return "Ɓ";
    if (x == "LTC")
        return "Ł";
    if (x == "EOS")
        return "ε";
    if (x == "ADA")
        return "₳";
    if (x == "XTZ")
        return "ꜩ";
    if (x == "XMR")
        return "ɱ";
    if (x == "ETC")
        return "ξ";
    if (x == "MKR")
        return "Μ";
    if (x == "ZEC")
        return "ⓩ";
    if (x == "DOGE")
        return "Ð";
    if (x == "REP")
        return "Ɍ";
    if (x == "REPV2")
        return "Ɍ";
    if (x == "STEEM")
        return "ȿ";
    if (x == "JPY")
        return "¥";
    if (x == "CAD")
        return "$";
    if (x == "CHF")
        return "₣";
    if (x == "AUD")
        return "$";
    return "_";
}

/// Returns true if the currency is directly related to USD or is USD.
/** Used to determine rounding on Value column. */
[[nodiscard]] inline auto is_USD_like(std::string_view x) -> bool
{
    return (x == "USD" || x == "EUR" || x == "GBP" || x == "USDC" ||
            x == "USDT" || x == "CAD" || x == "CHF" || x == "AUD");
}

/// Inserts space at front of \p value to align decimal place to \p offset.
/** Should be applied after thousands separators are inserted. */
[[nodiscard]] inline auto align_decimal(std::string const& value,
                                        std::size_t offset) -> std::string
{
    auto const decimal = value.rfind('.');
    if (decimal == std::string::npos || decimal > offset)
        return value;
    auto result = std::string(offset - decimal, ' ');
    std::copy(std::cbegin(value), std::cend(value), std::back_inserter(result));
    return result;
}

}  // namespace crab
#endif  // CRAB_FORMAT_MONEY_HPP
