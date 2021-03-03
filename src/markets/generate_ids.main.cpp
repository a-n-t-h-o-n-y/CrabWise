#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "../asset.hpp"
#include "../currency_pair.hpp"
#include "network/check_response.hpp"
#include "network/https_socket.hpp"
#include "network/to_ptree.hpp"

namespace {

[[nodiscard]] auto parse_crypto_symbol(std::string const& display_symbol)
    -> crab::Currency_pair
{
    auto const div_pos = display_symbol.find('/');
    return {display_symbol.substr(0, div_pos),
            display_symbol.substr(div_pos + 1)};
}

[[nodiscard]] auto build_rest_query(std::string const& key,
                                    std::string resource) -> std::string
{
    if (!resource.empty() && (resource.find('?') == std::string::npos))
        resource.push_back('?');
    else
        resource.push_back('&');
    return "/api/v1/" + resource + "token=" + key;
}

/// Query for list of crypto exchanges.
[[nodiscard]] auto get_exchanges_list(crab::HTTPS_socket& sock,
                                      std::string const& key)
    -> std::vector<std::string>
{
    auto const message = sock.get(build_rest_query(key, "crypto/exchange"));
    crab::check_response(message, "Finnhub - Failed to read crypto exchanges");
    auto result = std::vector<std::string>{};
    for (auto const& node : crab::to_ptree(message.body))
        result.push_back(node.second.get<std::string>(""));
    return result;
}

/// Return list of pairs of symbol_ids and Assets the given \p exchange has.
[[nodiscard]] auto exchange_assets(crab::HTTPS_socket& sock,
                                   std::string const& key,
                                   std::string const& exchange)
    -> std::vector<std::pair<std::string, crab::Asset>>
{
    auto const message =
        sock.get(build_rest_query(key, "crypto/symbol?exchange=" + exchange));
    crab::check_response(
        message, "Finnhub - Failed to read crypto symbols for " + exchange);
    auto result = std::vector<std::pair<std::string, crab::Asset>>{};
    for (auto const& node : crab::to_ptree(message.body)) {
        result.push_back(
            {node.second.get<std::string>("symbol"),
             crab::Asset{exchange,
                         parse_crypto_symbol(
                             node.second.get<std::string>("displaySymbol"))}});
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

/// Output one asset/id pair to \p os.
void make_get_id_line(std::ostream& os,
                      crab::Asset const& asset,
                      std::string const& id)
{
    os << "{{" << quote(asset.exchange) << ", {" << quote(asset.currency.base)
       << ", " << quote(asset.currency.quote) << "}}, " << quote(id) << "},\n";
}

void make_get_asset_line(std::ostream& os,
                         std::string const& id,
                         crab::Asset const& asset)
{
    os << "{" << quote(id) << ", {" << quote(asset.exchange) << ", {"
       << quote(asset.currency.base) << ", " << quote(asset.currency.quote)
       << "}}},\n";
}

/// Query for all symbol_ids from Finnhub and write it out to given cpp
/// file.
void generate_finnhub_symbol_id_tables(crab::HTTPS_socket& sock,
                                       std::string const& key,
                                       std::ostream& os)
{
    auto ids_and_assets = std::vector<std::pair<std::string, crab::Asset>>{};
    for (std::string const& exchange : get_exchanges_list(sock, key))
        append(ids_and_assets, exchange_assets(sock, key, exchange));

    os << "std::map<crab::Asset, std::string> get_id_map = {\n";
    /// get_id_map
    for (auto const& [id, asset] : ids_and_assets)
        make_get_id_line(os, asset, id);
    os << "};\n\n";

    /// get_asset_map
    os << "std::map<std::string, crab::Asset> get_asset_map = {\n";
    for (auto const& [id, asset] : ids_and_assets)
        make_get_asset_line(os, id, asset);
    os << "};\n";
}

}  // namespace

/// Generate .cpp maps of Assets and Finnhub 'symbol_id's.
int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr
            << "Error: Finnhub Key needs to be provided at command line.\n";
        return 1;
    }
    auto const key = std::string{argv[1]};
    auto sock      = crab::HTTPS_socket{};
    sock.connect("finnhub.io");
    generate_finnhub_symbol_id_tables(sock, key, std::cout);
    sock.disconnect();
    return 0;
}
