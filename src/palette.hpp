#ifndef CRAB_PALETTE_HPP
#define CRAB_PALETTE_HPP
#include <termox/painter/color.hpp>

namespace crab {

auto constexpr Background   = ox::Color::Background;
auto constexpr Red          = ox::Color{1};
auto constexpr Green        = ox::Color{2};
auto constexpr Almost_bg    = ox::Color{3};
auto constexpr Light_gray   = ox::Color{4};
auto constexpr Yellow       = ox::Color{5};
auto constexpr Gray         = ox::Color{6};
auto constexpr Foreground   = ox::Color::Foreground;
auto constexpr Scrollbar_bg = ox::Color{8};

auto const palette = ox::Palette{
    // clang-format off

    {Background,   ox::RGB{0x0b2940}},
    {Red,          ox::RGB{0xe08081}},
    {Green,        ox::RGB{0x2cdb63}},
    {Almost_bg,    ox::RGB{0x153a57}},
    {Light_gray,   ox::RGB{0x77b3e0}},
    {Yellow,       ox::RGB{0xdad45e}},
    {Gray,         ox::RGB{0x4282b3}},
    {Foreground,   ox::RGB{0xffffff}},
    {Scrollbar_bg, ox::RGB{0x1a1a1a}},

    // clang-format on
};

}  // namespace crab
#endif  // CRAB_PALETTE_HPP
