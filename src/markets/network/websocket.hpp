#ifndef CRAB_MARKETS_NETWORK_WEBSOCKET_HPP
#define CRAB_MARKETS_NETWORK_WEBSOCKET_HPP
#include <memory>
#include <string>

#include <boost/beast/websocket/stream.hpp>

#include "io_context.hpp"
#include "ssl.hpp"

namespace crab {

/// Secure websocket.
class Websocket {
   public:
    /// Perform connect, ssl handshake and websocket handshake.
    /** throws Crab_error if fails. */
    void connect(std::string const& host,
                 std::string const& URI  = "/",
                 std::string const& port = "443");

    /// Safely disconnect the socket.
    /** throws Crab_error if fails. */
    void disconnect();

    /// Send \p request to the socket.
    /** throws Crab_error if fails. */
    void write(std::string const& request);

    /// Perform one read from the socket, returning the read bytes.
    /** throws Crab_error if fails. */
    [[nodiscard]] auto read() -> std::string;

    /// Return true if connect has been called and disconnect has not.
    [[nodiscard]] auto is_connected() const -> bool { return connected_; }

   private:
    Context_t ssl_ctx_ = make_context();

    using Socket_t = boost::beast::websocket::stream<SSL_socket_t>;
    std::unique_ptr<Socket_t> socket_ =
        std::make_unique<Socket_t>(io_context(), ssl_ctx_);

    bool connected_ = false;
};

}  // namespace crab
#endif  // CRAB_MARKETS_NETWORK_WEBSOCKET_HPP
