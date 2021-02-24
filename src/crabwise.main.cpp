#include <termox/termox.hpp>

// 🦀
class Crabwise : public ox::layout::Vertical<> {
   public:
    ox::Titlebar& title = this->make_child<ox::Titlebar>(U"Crabwise");
    ox::Textbox& tb     = this->make_child<ox::Textbox>(U"Stonks");
};

int main() { return ox::System{}.run<Crabwise>(); }
