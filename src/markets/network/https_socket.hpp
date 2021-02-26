#ifndef CRAB_MARKETS_NETWORK_HTTPS_SOCKET_HPP
#define CRAB_MARKETS_NETWORK_HTTPS_SOCKET_HPP
#include <memory>
#include <string>

#include <boost/beast/core/flat_buffer.hpp>

#include "ssl.hpp"

namespace crab {

/// Connect to endpoint, send GET requests. Hardcoded port 443.
class HTTPS_socket {
   public:
    ~HTTPS_socket() { this->disconnect(); }

    /// Make Connection to \p host. \p host should have 'www.' prefix.
    /** Throws Crab_error if fails. */
    void connect(std::string const& host);

    /// Disconnect from endpoint. No-op if already disconnected.
    /** Throws Crab_error if fails. */
    void disconnect();

    /// Reconnects to host given in last connect() call.
    /** Throws Crab_error if fails. */
    void reconnect();

    /// HTTP Response, includes return code and string contents of body.
    struct Response {
        unsigned code;
        std::string body;
    };

    /// Send GET HTTP message to \p resource at endpoint. Return body.
    auto get(std::string const& resource) -> Response;

   private:
    Context_t ssl_ctx_{make_context()};

    std::unique_ptr<Socket_t> socket_{nullptr};

    boost::beast::flat_buffer buffer_;
    std::string host_;
};

}  // namespace crab
#endif  // CRAB_MARKETS_NETWORK_HTTPS_SOCKET_HPP
