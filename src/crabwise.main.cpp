#include <termox/termox.hpp>

#include "crabwise.hpp"

int main()
{
    return ox::System{ox::Mouse_mode::Drag}.run<crab::Crabwise>("assets.txt");
}
