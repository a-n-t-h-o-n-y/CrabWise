#include <termox/termox.hpp>

#include "crabwise.hpp"

int main() { return ox::System{}.run<crab::Crabwise>(); }

// namespace crab {

// class Labels
//     : public ox::HTuple<ox::HLabel, ox::HLabel, ox::HLabel, ox::HLabel> {
//    public:
//     Labels()
//     {
//         *this | ox::pipe::fixed_height(1);
//         this->get<0>().set_text(U"Currency" | ox::Trait::Underline);
//         this->get<1>().set_text(U"Last Price" | ox::Trait::Underline);
//         this->get<2>().set_text(U"Opening Price" | ox::Trait::Underline);
//         this->get<3>().set_text(U"Percent Change" | ox::Trait::Underline);
//     }
// };

// // TODO Format a bit nicer with quote currency spacing or longer names
// //      and symbols $ and %
// // (base/quote) (last price) (opening price) (percent change)
// class Currency_pair_display
//     : public ox::HTuple<ox::HLabel, ox::HLabel, ox::HLabel, ox::HLabel> {
//    public:
//     ox::HLabel& currency       = this->get<0>();  // longer names?
//     ox::HLabel& last_price     = this->get<1>();
//     ox::HLabel& opening_price  = this->get<2>();
//     ox::HLabel& percent_change = this->get<3>();

//    public:
//     Currency_pair_display() { *this | ox::pipe::fixed_height(1); }

//    public:
//     void set_opening_price(std::string value)
//     {
//         opening_price_ = std::stod(value);
//         insert_thousands_separators(value);
//         opening_price.set_text(value);
//         percent_change.set_text(
//             std::to_string(calc_percent_changed(last_price_,
//             opening_price_)));
//     }

//     void set_last_price(std::string value)
//     {
//         last_price_ = std::stod(value);
//         insert_thousands_separators(value);
//         last_price.set_text(value);
//         percent_change.set_text(
//             std::to_string(calc_percent_changed(last_price_,
//             opening_price_)));
//     }

//     void set_currency(std::string const& base, std::string const& quote)
//     {
//         auto display = ox::Glyph_string{base} | ox::Trait::Bold;
//         display.append(U' ');
//         display.append(quote);
//         currency.set_text(display);
//     }

//    private:
//     // These are only used for percentage calculation, so double is fine.
//     double last_price_    = 0;
//     double opening_price_ = 0;

//    private:
//     static auto calc_percent_changed(double current, double opening) ->
//     double
//     {
//         if (opening == 0.)
//             return 0.;
//         return 100. * ((current - opening) / opening);
//     }
// };

// class Coinbase_widget
//     : public ox::Passive<ox::VArray<Currency_pair_display, 11>> {
//    public:
//     Currency_pair_display& algo = this->get<0>();
//     Currency_pair_display& btc  = this->get<1>();
//     Currency_pair_display& xlm  = this->get<2>();
//     Currency_pair_display& eth  = this->get<3>();
//     Currency_pair_display& nu   = this->get<4>();
//     Currency_pair_display& comp = this->get<5>();
//     Currency_pair_display& mkr  = this->get<6>();
//     Currency_pair_display& cgld = this->get<7>();
//     Currency_pair_display& grt  = this->get<8>();
//     Currency_pair_display& aave = this->get<9>();
//     Currency_pair_display& fwb  = this->get<10>();

//    public:
//     Coinbase_widget()
//     {
//         algo.set_currency("ALGO", "USD");
//         btc.set_currency("BTC", "USD");
//         xlm.set_currency("XLM", "USD");
//         eth.set_currency("ETH", "USD");
//         nu.set_currency("NU", "USD");
//         comp.set_currency("COMP", "USD");
//         mkr.set_currency("MKR", "USD");
//         cgld.set_currency("CGLD", "USD");
//         grt.set_currency("GRT", "USD");
//         aave.set_currency("AAVE", "USD");
//         fwb.set_currency("FWB", "USD");

//         market_.make_https_connection();
//         (void)market_.currency_pairs();
//         std::cerr << market_.opening_price({"BTC", "USD"}).value << '\n';
//         market_.disconnect_https();

//         // auto const current_prices = market_.initialize();
//         // for (auto const& price : current_prices)
//         //     this->update_price(price);

//         // auto const opening_prices = market_.opening_prices();
//         // for (auto const& price : opening_prices)
//         //     this->set_opening_price(price);

//         // loop_.run_async([this](ox::Event_queue& q) {
//         //     auto const p = market_.read();
//         //     q.append(ox::Custom_event{[this, p] { this->update_price(p);
//         }});
//         // });
//     }

//    private:
//     Coinbase market_;
//     ox::Event_loop loop_;

//    private:
//     void set_opening_price(Price const& price)
//     {
//         try {
//             find_display(price.currency.base).set_opening_price(price.value);
//         }
//         catch (...) {
//         }
//     }

//     void update_price(Price const& price)
//     {
//         try {
//             find_display(price.currency.base).set_last_price(price.value);
//         }
//         catch (...) {
//         }
//     }

//    private:
//     auto find_display(std::string const& base) -> Currency_pair_display&
//     {
//         if (base == "ALGO")
//             return algo;
//         else if (base == "BTC")
//             return btc;
//         else if (base == "XLM")
//             return xlm;
//         else if (base == "ETH")
//             return eth;
//         else if (base == "NU")
//             return nu;
//         else if (base == "COMP")
//             return comp;
//         else if (base == "MKR")
//             return mkr;
//         else if (base == "CGLD")
//             return cgld;
//         else if (base == "GRT")
//             return grt;
//         else if (base == "AAVE")
//             return aave;
//         else if (base == "FWB")
//             return fwb;
//         throw std::runtime_error{
//             "Coinbase_widget::find_display: Base Current not recognized: " +
//             base};
//     }
// };

// // 🦀
// class Crabwise : public ox::layout::Vertical<> {
//    public:
//     Crabwise() { ox::Terminal::set_palette(ox::en4::palette); }

//    public:
//     ox::Titlebar& title       = this->make_child<ox::Titlebar>(U"CrabWise");
//     Labels& labels            = this->make_child<Labels>();
//     Coinbase_widget& coinbase = this->make_child<Coinbase_widget>();
//     Widget& buffer            = this->make_child();
// };

// }  // namespace crab
