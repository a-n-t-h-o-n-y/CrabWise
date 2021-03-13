#ifndef CRAB_CURRENCY_DISPLAY_HPP
#define CRAB_CURRENCY_DISPLAY_HPP
#include <string>
#include <utility>

#include <termox/termox.hpp>

#include "format_money.hpp"

namespace crab {

/// Settable as a string abbriviation, displayed in its symbolic representation.
class Currency_display : public ox::HLabel {
   public:
    Currency_display(std::string x = "")
    {
        using namespace ox::pipe;
        *this | fixed_width(3) | fixed_height(1) | align_center();
        if (!x.empty())
            this->set(std::move(x));
    }

   public:
    /// Set the currency to display, \p x is a string abbriviation.
    /** Uses an underscore if no symbol found for the given currency. */
    void set(std::string x)
    {
        currency_ = std::move(x);
        this->ox::HLabel::set_text(currency_to_symbol(currency_));
    }

    /// Return the string abbriviation currency was set with.
    [[nodiscard]] auto currency() const -> std::string const&
    {
        return currency_;
    }

   private:
    std::string currency_;
};

}  // namespace crab
#endif  // CRAB_CURRENCY_DISPLAY_HPP
