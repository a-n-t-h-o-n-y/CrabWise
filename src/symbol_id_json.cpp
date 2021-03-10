#include "symbol_id_json.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <ntwk/check_response.hpp>
#include <ntwk/https_socket.hpp>

#include <simdjson.h>

#include "asset.hpp"
#include "currency_pair.hpp"
#include "filenames.hpp"
#include "filesystem.hpp"
#include "ntwk/https_socket.hpp"

namespace {

using JSON_element_t = simdjson::simdjson_result<simdjson::dom::element>;

[[nodiscard]] auto json_parser() -> simdjson::dom::parser&
{
    static auto parser = simdjson::dom::parser{};
    return parser;
}

[[nodiscard]] auto build_rest_query(std::string resource,
                                    std::string const& key) -> std::string
{
    if (!resource.empty() && (resource.find('?') == std::string::npos))
        resource.push_back('?');
    else
        resource.push_back('&');
    return "/api/v1/" + resource + "token=" + key;
}

[[nodiscard]] auto parse_crypto_symbol(std::string const& display_symbol)
    -> crab::Currency_pair
{
    auto const div_pos = display_symbol.find('/');
    return {display_symbol.substr(0, div_pos),
            display_symbol.substr(div_pos + 1)};
}

/// Query for list of crypto exchanges.
[[nodiscard]] auto get_exchanges_list(ntwk::HTTPS_socket& sock,
                                      std::string const& key)
    -> std::vector<std::string>
{
    auto const message = sock.get(build_rest_query("crypto/exchange", key));
    ntwk::check_response(message, "Finnhub - Failed to read crypto exchanges");
    auto result = std::vector<std::string>{};
    for (auto const exchange : json_parser().parse(message.body)) {
        auto const x = (std::string)exchange;
        if (x != "BITTREX" && x != "OKEX")  // Do not seem to work w/finnhub
            result.push_back(x);
    }
    return result;
}

/// Return list of pairs of symbol_ids and Assets the given \p exchange has.
[[nodiscard]] auto exchange_assets(ntwk::HTTPS_socket& sock,
                                   std::string const& key,
                                   std::string const& exchange)
    -> std::vector<std::pair<std::string, crab::Asset>>
{
    auto const message =
        sock.get(build_rest_query("crypto/symbol?exchange=" + exchange, key));
    ntwk::check_response(
        message, "Finnhub - Failed to read crypto symbols for " + exchange);
    auto result = std::vector<std::pair<std::string, crab::Asset>>{};
    for (auto const& node : json_parser().parse(message.body)) {
        result.push_back(
            {(std::string)node["symbol"],
             crab::Asset{exchange, parse_crypto_symbol(
                                       (std::string)node["displaySymbol"])}});
    }
    return result;
}

template <typename Container_t>
void append(Container_t& to, Container_t&& from)
{
    std::move(std::begin(from), std::end(from), std::back_inserter(to));
}

[[nodiscard]] auto quote(std::string const& x) -> std::string
{
    return '\"' + x + '\"';
};

/// Write single entry of json array, id and asset.
void to_json(std::ostream& os, std::string const& id, crab::Asset const& asset)
{
    os << '{';
    os << quote("i") << ':' << quote(id) << ',';
    os << quote("x") << ':' << quote(asset.exchange) << ',';
    os << quote("b") << ':' << quote(asset.currency.base) << ',';
    os << quote("q") << ':' << quote(asset.currency.quote);
    os << "}";
}

void generate_finnhub_symbol_id_json(ntwk::HTTPS_socket& sock,
                                     std::string const& key,
                                     std::ostream& os)
{
    auto ids_and_assets = std::vector<std::pair<std::string, crab::Asset>>{};
    for (std::string const& exchange : get_exchanges_list(sock, key))
        append(ids_and_assets, exchange_assets(sock, key, exchange));

    os << '{' << quote("data") << ":[";
    auto div = "";
    for (auto const& [id, asset] : ids_and_assets) {
        os << div;
        div = ",";
        to_json(os, id, asset);
    }
    os << "]}";
}

auto make_socket_connection() -> ntwk::HTTPS_socket
{
    auto sock = ntwk::HTTPS_socket{};
    sock.connect("finnhub.io");
    return sock;
}

/// Read finnhub key, assumes it exists and is valid.
auto read_key() -> std::string
{
    auto file = std::ifstream{crab::finnhub_key_filepath()};
    auto key  = std::string{};
    file >> key;
    return key;
}

}  // namespace

namespace crab {
void write_ids_json()
{
    auto sock      = make_socket_connection();
    auto const key = read_key();
    auto file      = std::ofstream{crab::symbol_ids_json_filepath()};
    generate_finnhub_symbol_id_json(sock, key, file);
    sock.disconnect();
}

auto read_ids_json(fs::path const& filepath)
    -> std::vector<std::pair<std::string, Asset>>
{
    auto doc    = json_parser().load(filepath.string());
    auto result = std::vector<std::pair<std::string, Asset>>{};
    for (auto const& p : doc["data"]) {
        result.push_back({(std::string)p["i"],
                          {(std::string)p["x"],
                           {(std::string)p["b"], (std::string)p["q"]}}});
    }
    return result;
}

}  // namespace crab
