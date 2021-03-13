#ifndef CRAB_LINE_HPP
#define CRAB_LINE_HPP
#include <termox/termox.hpp>

#include "palette.hpp"

namespace crab {

// TODO possibly move into TermOx

/// Simple Horizontal line element used throughout app.
class HLine : public ox::Widget {
   public:
    HLine()
    {
        *this | ox::pipe::wallpaper(U'─' | fg(crab::Gray)) |
            ox::pipe::fixed_height(1);
    }
};

/// Simple Vertical line element used throughout app.
class VLine : public ox::Widget {
   public:
    VLine()
    {
        *this | ox::pipe::wallpaper(U'│' | fg(crab::Gray)) |
            ox::pipe::fixed_width(1);
    }
};

}  // namespace crab
#endif  // CRAB_LINE_HPP
