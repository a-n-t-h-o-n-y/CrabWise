#ifndef CRAB_PALETTE_HPP
#define CRAB_PALETTE_HPP
#include <termox/painter/color.hpp>

namespace crab {

auto constexpr Background = ox::Color::Background;
auto constexpr Red        = ox::Color{1};
auto constexpr Green      = ox::Color{2};
// auto constexpr Olive   = ox::Color{3};
auto constexpr Light_gray   = ox::Color{4};
// auto constexpr Black        = ox::Color{5};
auto constexpr Gray         = ox::Color{6};
auto constexpr Foreground   = ox::Color::Foreground;
auto constexpr Scrollbar_bg = ox::Color{8};

auto const palette = ox::Palette{
    // clang-format off

    {Background,   ox::RGB{0x0b, 0x29, 0x40}},
    {Red,          ox::RGB{0xe0, 0x80, 0x81}},
    {Green,        ox::RGB{0x2c, 0xdb, 0x63}},
    {Light_gray,   ox::RGB{0x77, 0xb3, 0xe0}},
    {Gray,         ox::RGB{0x42, 0x82, 0xb3}},
    {Foreground,   ox::RGB{0xff, 0xff, 0xff}},
    {Scrollbar_bg, ox::RGB{0x1a, 0x1a, 0x1a}},

    // clang-format on
};

}  // namespace crab
#endif  // CRAB_PALETTE_HPP
