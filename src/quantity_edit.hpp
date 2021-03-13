#ifndef CRAB_QUANTITY_EDIT_HPP
#define CRAB_QUANTITY_EDIT_HPP
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <iterator>
#include <optional>
#include <string>
#include <type_traits>

#include <termox/termox.hpp>

#include "format_money.hpp"
#include "palette.hpp"

namespace crab {

class Quantity_edit : public ox::Textbox {
   public:
    sl::Signal<void(double)> quantity_updated;

   public:
    Quantity_edit()
    {
        *this | ox::pipe::fixed_height(1) | bg(crab::Almost_bg);
        this->disable_scrollwheel();
        this->initialize(0.);
    }

    void initialize(double value)
    {
        auto display = std::to_string(value);
        if (value == 0.)
            display = "0";
        else {
            format_decimal_zeros(display);
            insert_thousands_separators(display);
        }
        this->set_contents(display);
        quantity_updated.emit(value);
        value_ = value;
    }

    // TODO Change to as_double?
    auto quantity() const -> double { return value_; }

   protected:
    auto key_press_event(ox::Key k) -> bool override
    {
        switch (k) {
            case ox::Key::Backspace:
            case ox::Key::Backspace_1:
            case ox::Key::Backspace_2:
            case ox::Key::Delete: {
                auto const result = Textbox::key_press_event(k);
                if (auto const dbl = get_double(this->contents().str()); dbl) {
                    value_ = *dbl;
                    quantity_updated.emit(value_);
                }
                return result;
            }
            case ox::Key::Arrow_right:
            case ox::Key::Arrow_left:
            case ox::Key::Arrow_up:
            case ox::Key::Arrow_down: return Textbox::key_press_event(k);
            default: break;
        }
        if (static_cast<std::underlying_type_t<ox::Key>>(k) > 127)
            return true;

        auto const ch = key_to_char(k);
        if (validate_char(ch)) {
            if (auto const dbl = get_double(this->generate_string(ch)); dbl) {
                value_ = *dbl;
                quantity_updated.emit(value_);
                return Textbox::key_press_event(k);
            }
        }
        return true;
    }

   private:
    /// Generate the string that will be made once the char is commited.
    /** Used for validation before sending the char to the Textbox base class */
    auto generate_string(char c) -> std::string
    {
        auto result = this->contents().str();
        result.insert(this->cursor_index(), 1, c);
        return result;
    }

   private:
    [[nodiscard]] static auto validate_char(char c) -> bool
    {
        return std::isdigit(c) || c == ',' || c == '.';
    }

    /// Returns parsed double if valid, std::nullopt if not valid.
    [[nodiscard]] static auto get_double(std::string input)
        -> std::optional<double>
    {
        input.erase(std::remove(std::begin(input), std::end(input), ','),
                    std::end(input));
        if (input.empty() || input == ".")
            return 0.;
        auto count = std::size_t{0};
        try {
            auto const result = std::stod(input, &count);
            if (count == input.size())
                return result;
            else
                return std::nullopt;
        }
        catch (std::exception const&) {
            return std::nullopt;
        }
    }

   private:
    double value_ = 0;
};

}  // namespace crab
#endif  // CRAB_QUANTITY_EDIT_HPP
