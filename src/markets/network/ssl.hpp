#ifndef CRAB_MARKETS_NETWORK_SSL_HPP
#define CRAB_MARKETS_NETWORK_SSL_HPP
#include <exception>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/context_base.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/stream_base.hpp>
#include <boost/asio/ssl/verify_mode.hpp>

#include "../error.hpp"

namespace crab {

using Context_t = boost::asio::ssl::context;
using Socket_t  = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

inline auto set_hostname(Socket_t& socket, std::string const& host) -> bool
{
    return SSL_set_tlsext_host_name(socket.native_handle(), host.c_str());
}

/// Create a new SSL_context object, with verification method set.
inline auto make_context() -> Context_t
{
    auto context = Context_t{Context_t::sslv23_client};
    context.set_options(Context_t::default_workarounds | Context_t::no_sslv3);
    context.set_verify_mode(boost::asio::ssl::verify_peer);
    context.set_default_verify_paths();
    return context;
}

/// Perform SSL handshake as client.
/** throws Crab_error on failure. */
inline void handshake(Socket_t& socket)
{
    try {
        socket.handshake(boost::asio::ssl::stream_base::client);
    }
    catch (std::exception const& e) {
        throw Crab_error{std::string{"network::ssl::handshake: "} + e.what()};
    }
}

}  // namespace crab
#endif  // CRAB_MARKETS_NETWORK_SSL_HPP
