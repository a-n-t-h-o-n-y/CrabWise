#ifndef CRAB_AMOUNT_DISPLAY_HPP
#define CRAB_AMOUNT_DISPLAY_HPP
#include <cstddef>
#include <string>
#include <utility>

#include <termox/termox.hpp>

#include "format_money.hpp"

namespace crab {

/// Displays an amount passed in as a string or double.
/** Inserts thousands separators and formats decimal zeros if needed. */
class Amount_display : public ox::HLabel {
   public:
    sl::Signal<void(double)> amount_updated;

   public:
    void set(double amount)
    {
        double_ = amount;
        string_ = std::to_string(amount);
        format_decimal_zeros(string_);
        insert_thousands_separators(string_);
        this->ox::HLabel::set_text(string_);
        amount_updated.emit(double_);
    }

    void set(std::string amount)
    {
        string_ = std::move(amount);
        double_ = std::stod(string_);
        format_decimal_zeros(string_);
        insert_thousands_separators(string_);
        this->ox::HLabel::set_text(string_);
        amount_updated.emit(double_);
    }

    /// Return set value as a double.
    [[nodiscard]] auto as_double() const -> double { return double_; }

    /// Return set value as it was passed in, without formatting.
    [[nodiscard]] auto as_string() const -> std::string const&
    {
        return string_;
    }

   private:
    double double_;
    std::string string_;
};

/// Displays an amount passed in as a string or double, aligns on decimal place.
/** Inserts thousands separators and formats decimal zeros if needed, \p offset
 *  is used for decimal alignment, if \p hundredths_round is true, rounds. */
class Aligned_amount_display : public ox::HLabel {
   public:
    sl::Signal<void(double)> amount_updated;

   public:
    void set(double amount)
    {
        double_ = amount;
        if (round_hundredths_)
            string_ = round_and_to_string(amount, 2);
        else
            string_ = std::to_string(amount);
        format_decimal_zeros(string_);
        insert_thousands_separators(string_);
        string_ = align_decimal(string_, offset_);
        this->ox::HLabel::set_text(string_);
        amount_updated.emit(double_);
    }

    void set(std::string amount) { this->set(std::stod(amount)); }

    void set_offset(std::size_t x)
    {
        offset_ = x;
        if (!string_.empty())
            this->set(string_);
    }

    void round_to_hundredths(bool x)
    {
        round_hundredths_ = x;
        if (!string_.empty())
            this->set(string_);
    }

    /// Return set value as a double.
    [[nodiscard]] auto as_double() const -> double { return double_; }

    /// Return set value as it was passed in, without formatting.
    [[nodiscard]] auto as_string() const -> std::string const&
    {
        return string_;
    }

   private:
    double double_;
    std::string string_;
    std::size_t offset_    = 0;
    bool round_hundredths_ = false;
};
}  // namespace crab
#endif  // CRAB_AMOUNT_DISPLAY_HPP
