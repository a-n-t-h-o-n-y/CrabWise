#ifndef CRAB_LINE_HPP
#define CRAB_LINE_HPP
#include <termox/termox.hpp>

#include "palette.hpp"

namespace crab {

/// Simple Horizontal line element used throughout app.
class Line : public ox::Widget {
   public:
    Line()
    {
        *this | ox::pipe::wallpaper(U'─' | fg(crab::Gray)) |
            ox::pipe::fixed_height(1);
    }
};

}  // namespace crab
#endif  // CRAB_LINE_HPP
