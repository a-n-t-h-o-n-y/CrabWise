#include "poloniex.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <socket/make_symbol.hpp>
#include <socket/message.hpp>

#include <utility/decimal.hpp>
#include <utility/log.hpp>

#include "../network/https_socket.hpp"

namespace pt = boost::property_tree;

namespace {

/// Concise Exception Throwing
void error(std::string const& message) { throw std::runtime_error(message); }

/// Subscribe to all ticker price events.
template <typename Socket_t>
void subscribe_all(Socket_t& ws)
{
    auto constexpr ticker_channel = 1002;

    auto tree = pt::ptree{};
    tree.add("command", "subscribe");
    tree.add("channel", ticker_channel);

    auto oss = std::ostringstream{};
    write_json(oss, tree);
    ws.write(oss.str());
    ws.read();
}

/// Split a response up into its individual components.
auto split(std::string const& message) -> std::vector<std::string>
{
    auto result = std::vector<std::string>{};
    boost::split(result, message, boost::is_any_of("[],\" "));
    result.erase(std::remove_if(std::begin(result), std::end(result),
                                [](auto const& s) { return s.empty(); }),
                 std::end(result));
    return result;
}

auto is_price(std::vector<std::string> const& data) -> bool
{
    return data.size() > 2;
}

using Currency = sckt::Currency;

// Currency Pair
struct Pair {
    std::string_view ticker;
    Currency quote_currency;
};

auto const id_pairs_map = std::map<int, Pair>{
    /* clang-format off */
    /* -------- BTC --------  */
    {177, {"ARDR",   Currency::BTC}},
    {253, {"ATOM",   Currency::BTC}},
    {324, {"AVA",    Currency::BTC}},
    {210, {"BAT",    Currency::BTC}},
    {189, {"BCH",    Currency::BTC}},
    {236, {"BCHABC", Currency::BTC}},
    {238, {"BCHSV",  Currency::BTC}},
    {232, {"BNT",    Currency::BTC}},
    {14,  {"BTS",    Currency::BTC}},
    {15,  {"BURST",  Currency::BTC}},
    {333, {"CHR",    Currency::BTC}},
    {194, {"CVC",    Currency::BTC}},
    {24,  {"DASH",   Currency::BTC}},
    {162, {"DCR",    Currency::BTC}},
    {27,  {"DOGE",   Currency::BTC}},
    {201, {"EOS",    Currency::BTC}},
    {171, {"ETC",    Currency::BTC}},
    {148, {"ETH",    Currency::BTC}},
    {266, {"ETHBNT", Currency::BTC}},
    {246, {"FOAM",   Currency::BTC}},
    {317, {"FXC",    Currency::BTC}},
    {198, {"GAS",    Currency::BTC}},
    {185, {"GNT",    Currency::BTC}},
    {251, {"GRIN",   Currency::BTC}},
    {43,  {"HUC",    Currency::BTC}},
    {207, {"KNC",    Currency::BTC}},
    {275, {"LINK",   Currency::BTC}},
    {213, {"LOOM",   Currency::BTC}},
    {250, {"LPT",    Currency::BTC}},
    {163, {"LSK",    Currency::BTC}},
    {50,  {"LTC",    Currency::BTC}},
    {229, {"MANA",   Currency::BTC}},
    {295, {"MATIC",  Currency::BTC}},
    {302, {"MKR",    Currency::BTC}},
    {309, {"NEO",    Currency::BTC}},
    {64,  {"NMC",    Currency::BTC}},
    {248,  {"NMR",   Currency::BTC}},
    {69,  {"NXT",    Currency::BTC}},
    {196, {"OMG",    Currency::BTC}},
    {249, {"POLY",   Currency::BTC}},
    {75,  {"PPC",    Currency::BTC}},
    {221, {"QTUM",   Currency::BTC}},
    {174, {"REP",    Currency::BTC}},
    {170, {"SBD",    Currency::BTC}},
    {150, {"SC",     Currency::BTC}},
    {204, {"SNT",    Currency::BTC}},
    {290, {"SNX",    Currency::BTC}},
    {168, {"STEEM",  Currency::BTC}},
    {200, {"STORJ",  Currency::BTC}},
    {89,  {"STR",    Currency::BTC}},
    {182, {"STRAT",  Currency::BTC}},
    {312, {"SWFTC",  Currency::BTC}},
    {92,  {"SYS",    Currency::BTC}},
    {263, {"TRX",    Currency::BTC}},
    {108, {"XCP",    Currency::BTC}},
    {112, {"XEM",    Currency::BTC}},
    {114, {"XMR",    Currency::BTC}},
    {117, {"XRP",    Currency::BTC}},
    {277, {"XTZ",    Currency::BTC}},
    {178, {"ZEC",    Currency::BTC}},
    {192, {"ZRX",    Currency::BTC}},
    {342, {"MDT",    Currency::BTC}},

    /* -------- BNB --------  */
    {336, {"BTC", Currency::BNB}},

    /* -------- BUSD --------  */
    {340, {"BNB", Currency::BUSD}},
    {341, {"BTC", Currency::BUSD}},

    /* -------- DAI --------  */
    {306, {"BTC", Currency::DAI}},
    {307, {"ETH", Currency::DAI}},

    /* -------- ETH --------  */
    {211, {"BAT", Currency::ETH}},
    {190, {"BCH", Currency::ETH}},
    {202, {"EOS", Currency::ETH}},
    {172, {"ETC", Currency::ETH}},
    {176, {"REP", Currency::ETH}},
    {179, {"ZEC", Currency::ETH}},
    {193, {"ZRX", Currency::ETH}},

    /* -------- PAX --------  */
    {284, {"BTC", Currency::PAX}},
    {285, {"ETH", Currency::PAX}},

    /* -------- TRX --------  */
    {326, {"AVA",   Currency::TRX}},
    {271, {"BTT",   Currency::TRX}},
    {335, {"CHR",   Currency::TRX}},
    {267, {"ETH",   Currency::TRX}},
    {319, {"FXC",   Currency::TRX}},
    {316, {"JST",   Currency::TRX}},
    {276, {"LINK",  Currency::TRX}},
    {297, {"MATIC", Currency::TRX}},
    {311, {"NEO",   Currency::TRX}},
    {292, {"SNX",   Currency::TRX}},
    {274, {"STEEM", Currency::TRX}},
    {314, {"SWFTC", Currency::TRX}},
    {273, {"WIN",   Currency::TRX}},
    {268, {"XRP",   Currency::TRX}},
    {279, {"XTZ",   Currency::TRX}},
    {339, {"BNB",   Currency::TRX}},
    {344, {"MDT",   Currency::TRX}},

    /* -------- USDC --------  */
    {254, {"ATOM",   Currency::USDC}},
    {235, {"BCH",    Currency::USDC}},
    {237, {"BCHABC", Currency::USDC}},
    {239, {"BCHSV",  Currency::USDC}},
    {224, {"BTC",    Currency::USDC}},
    {256, {"DASH",   Currency::USDC}},
    {243, {"DOGE",   Currency::USDC}},
    {257, {"EOS",    Currency::USDC}},
    {258, {"ETC",    Currency::USDC}},
    {225, {"ETH",    Currency::USDC}},
    {252, {"GRIN",   Currency::USDC}},
    {244, {"LTC",    Currency::USDC}},
    {242, {"STR",    Currency::USDC}},
    {264, {"TRX",    Currency::USDC}},
    {226, {"USDT",   Currency::USDC}},
    {241, {"XMR",    Currency::USDC}},
    {240, {"XRP",    Currency::USDC}},
    {245, {"ZEC",    Currency::USDC}},

    /* -------- USDT --------  */
    {288, {"BTC", Currency::USDJ}},
    {323, {"BTT", Currency::USDJ}},
    {289, {"TRX", Currency::USDJ}},

    /* -------- USDT --------  */
    {255, {"ATOM",     Currency::USDT}},
    {325, {"AVA",      Currency::USDT}},
    {212, {"BAT",      Currency::USDT}},
    {191, {"BCH",      Currency::USDT}},
    {260, {"BCHABC",   Currency::USDT}},
    {298, {"BCHBEAR",  Currency::USDT}},
    {259, {"BCHSV",    Currency::USDT}},
    {299, {"BCHBULL",  Currency::USDT}},
    {320, {"BCN",      Currency::USDT}},
    {280, {"BEAR",     Currency::USDT}},
    {293, {"BSVBEAR",  Currency::USDT}},
    {294, {"BSVBULL",  Currency::USDT}},
    {121, {"BTC",      Currency::USDT}},
    {270, {"BTT",      Currency::USDT}},
    {281, {"BULL",     Currency::USDT}},
    {304, {"BVOL",     Currency::USDT}},
    {334, {"CHR",      Currency::USDT}},
    {308, {"DAI",      Currency::USDT}},
    {122, {"DASH",     Currency::USDT}},
    {262, {"DGB",      Currency::USDT}},
    {216, {"DOGE",     Currency::USDT}},
    {203, {"EOS",      Currency::USDT}},
    {330, {"EOSBEAR",  Currency::USDT}},
    {329, {"EOSBULL",  Currency::USDT}},
    {173, {"ETC",      Currency::USDT}},
    {149, {"ETH",      Currency::USDT}},
    {300, {"ETHBEAR",  Currency::USDT}},
    {301, {"ETHBULL",  Currency::USDT}},
    {318, {"FXC",      Currency::USDT}},
    {217, {"GNT",      Currency::USDT}},
    {261, {"GRIN",     Currency::USDT}},
    {305, {"IBVOL",    Currency::USDT}},
    {315, {"JST",      Currency::USDT}},
    {322, {"LINK",     Currency::USDT}},
    {332, {"LINKBEAR", Currency::USDT}},
    {331, {"LINKBULL", Currency::USDT}},
    {218, {"LSK",      Currency::USDT}},
    {123, {"LTC",      Currency::USDT}},
    {231, {"MANA",     Currency::USDT}},
    {296, {"MATIC",    Currency::USDT}},
    {303, {"MKR",      Currency::USDT}},
    {310, {"NEO",      Currency::USDT}},
    {124, {"NXT",      Currency::USDT}},
    {286, {"PAX",      Currency::USDT}},
    {223, {"QTUM",     Currency::USDT}},
    {175, {"REP",      Currency::USDT}},
    {219, {"SC",       Currency::USDT}},
    {291, {"SNX",      Currency::USDT}},
    {321, {"STEEM",    Currency::USDT}},
    {125, {"STR",      Currency::USDT}},
    {313, {"SWFTC",    Currency::USDT}},
    {265, {"TRX",      Currency::USDT}},
    {282, {"TRXBEAR",  Currency::USDT}},
    {283, {"TRXBULL",  Currency::USDT}},
    {287, {"USDJ",     Currency::USDT}},
    {272, {"WIN",      Currency::USDT}},
    {126, {"XMR",      Currency::USDT}},
    {127, {"XRP",      Currency::USDT}},
    {328, {"XRPBEAR",  Currency::USDT}},
    {327, {"XRPBULL",  Currency::USDT}},
    {278, {"XTZ",      Currency::USDT}},
    {180, {"ZEC",      Currency::USDT}},
    {220, {"ZRX",      Currency::USDT}},
    {337, {"BNB",      Currency::USDT}},
    {338, {"BUSD",     Currency::USDT}},
    {343, {"MDT",      Currency::USDT}},

    /* Removed */
    /* {7,   {"BCN",    Currency::BTC}}, */
    /* {20,  {"CLAM",   Currency::BTC}}, */
    /* {25,  {"DGB",    Currency::BTC}}, */
    /* {155, {"FCT",    Currency::BTC}}, */
    /* {38,  {"GAME",   Currency::BTC}}, */
    /* {167, {"LBC",    Currency::BTC}}, */
    /* {51,  {"MAID",   Currency::BTC}}, */
    /* {61,  {"NAV",    Currency::BTC}}, */
    /* {58,  {"OMNI",   Currency::BTC}}, */
    /* {184, {"PASC",   Currency::BTC}}, */
    /* {97,  {"VIA",    Currency::BTC}}, */
    /* {100, {"VTC",    Currency::BTC}}, */
    /* {116, {"XPM",    Currency::BTC}}, */
    /* {269, {"BTT",    Currency::BTC}}, */
    /* clang-format on */
};

auto get_ticker(int id) -> std::string
{
    return std::string{id_pairs_map.at(id).ticker};
}

auto get_quote_currency(int id) -> Currency
{
    return id_pairs_map.at(id).quote_currency;
}

auto extract_symbol(std::vector<std::string> const& data) -> sckt::Symbol
{
    auto constexpr ticker_id_index = 2;
    auto const currency_pair_id    = std::stoi(data[ticker_id_index]);
    return sckt::make_symbol(sckt::MarketDataType::POLONIEX,
                             get_ticker(currency_pair_id),
                             get_quote_currency(currency_pair_id));
}

auto extract_price(std::vector<std::string> const& data) -> sckt::Price_t
{
    auto constexpr last_price_index = 3;
    return data[last_price_index];
}

auto build_price(int id, sckt::Price_t price) -> sckt::message::Last_price
{
    auto result       = sckt::message::Last_price{};
    result.symbol     = sckt::make_symbol(sckt::MarketDataType::POLONIEX,
                                      get_ticker(id), get_quote_currency(id));
    result.last_price = price;
    return result;
}

auto build_price(std::vector<std::string> const& data)
    -> sckt::message::Last_price
{
    auto result       = sckt::message::Last_price{};
    result.symbol     = extract_symbol(data);
    result.last_price = extract_price(data);
    return result;
}

template <typename Socket_t>
auto get_initial_prices_json(Socket_t& socket) -> std::string
{
    auto const response = socket.get("/public?command=returnTicker");
    if (response.code != 200) {
        error("poloniex.cpp::get_initial_prices_json\nReturn Code: " +
              std::to_string(response.code) + ' ' + response.body);
    }
    return response.body;
}

}  // namespace

namespace market_server::market {

void Poloniex::initialize()
{
    this->cache_initial_prices();
    ws_.response.connect([this](auto const& x) { this->parse_and_emit(x); });
    if (!ws_.connect(ws_endpoint_))
        error("Poloniex::initialize: Can't connect to " + ws_endpoint_);
    subscribe_all(ws_);
}

void Poloniex::reconnect()
{
    auto log = utility::Log::market{"Poloniex::reconnect()"};
    ws_.disconnect();
    if (!ws_.connect(ws_endpoint_)) {
        log << "Reconnect Failed";
        error("Poloniex::reconnect() Failed");
    }
    subscribe_all(ws_);
    log << "Poloniex::reconnect() Succeeded";
}

void Poloniex::parse_and_emit(std::string const& message)
{
    auto const words = split(message);
    if (!is_price(words))
        return;
    try {
        this->last_price(build_price(words));
    }
    catch (...) {
        utility::Log::market{"Poloniex::parse_and_emit: Error Building Price"}
            << message;
    }
}

void Poloniex::initial_price_parse_and_emit(pt::ptree const& root)
{
    for (auto const& ticker : root) {
        try {
            auto const id   = ticker.second.get("id", 0);
            auto const last = ticker.second.get("last", sckt::Price_t{"0"});
            this->last_price(build_price(id, last));
        }
        catch (...) {
            utility::Log::market{
                "Poloniex::initial_price_parse_and_emit: Parsing Error"}
                << "ID: " + std::to_string(ticker.second.get("id", -1));
        }
    }
}

void Poloniex::opening_price_parse_and_emit(pt::ptree const& root)
{
    for (auto const& ticker : root) {
        try {
            auto const id = ticker.second.get("id", 0);
            auto const last =
                utility::Decimal{ticker.second.get<sckt::Price_t>("last", "0")};
            auto const change = utility::Decimal{
                ticker.second.get<sckt::Price_t>("percentChange", "0")};
            auto const open  = last * ((-1 * change) + 1);
            auto const price = build_price(id, utility::to_string(open));
            this->opening_price({price.symbol, price.last_price});
        }
        catch (...) {
            utility::Log::market{
                "Poloniex::opening_price_parse_and_emit: Error Parsing"}
                << "ID: " + std::to_string(ticker.second.get("id", -1));
        }
    }
}

void Poloniex::cache_initial_prices()
{
    auto socket = network::HTTPS_socket{};
    if (!socket.connect(https_endpoint_)) {
        error(
            "Poloniex::cache_initial_prices\n Can't Connect to HTTPS Socket " +
            https_endpoint_);
    }
    auto root = pt::ptree{};
    {
        auto const json = get_initial_prices_json(socket);
        auto ss         = std::istringstream{json};
        read_json(ss, root);
    }
    this->initial_price_parse_and_emit(root);
    this->opening_price_parse_and_emit(root);
    socket.disconnect();
}

}  // namespace market_server::market
