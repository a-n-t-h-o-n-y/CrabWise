#include "https_socket.hpp"

#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>

#include <boost/asio/connect.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/system/error_code.hpp>

#include "../error.hpp"
#include "io_context.hpp"
#include "ssl.hpp"

namespace crab {

void HTTPS_socket::connect(std::string const& host)
{
    if (socket_ != nullptr)
        throw Crab_error{"HTTPS_socket::connect: socket_ is null."};

    auto temp_sock = std::make_unique<SSL_socket_t>(io_context(), ssl_ctx_);

    if (!SSL_set_tlsext_host_name(temp_sock->native_handle(), host.c_str()))
        throw Crab_error{"HTTPS_socket::connect: set_tlsext failed."};

    using tcp = boost::asio::ip::tcp;

    auto resolver        = tcp::resolver{io_context()};
    auto const endpoints = resolver.resolve(host, "443");
    boost::asio::connect(temp_sock->next_layer(), std::cbegin(endpoints),
                         std::cend(endpoints));
    handshake(*temp_sock);
    host_   = host;
    socket_ = std::move(temp_sock);
}

void HTTPS_socket::disconnect()
{
    if (socket_ == nullptr)
        return;
    {
        auto status = boost::system::error_code{};
        socket_->lowest_layer().cancel(status);
        if (status.failed()) {
            throw Crab_error{"HTTPS_socket::disconnect(): " + status.message()};
        }
    }
    {
        auto status = boost::system::error_code{};
        socket_->shutdown(status);
        if (status == boost::asio::error::eof)
            status.assign(0, status.category());
        if (status.failed() &&
            status != boost::asio::ssl::error::stream_truncated) {
            throw Crab_error{"HTTPS_socket::disconnect(): " + status.message()};
        }
    }
    socket_->lowest_layer().close();
    socket_ = nullptr;
}

void HTTPS_socket::reconnect()
{
    socket_ = nullptr;
    this->connect(host_);
}

auto HTTPS_socket::get(std::string const& resource) -> Response
{
    if (socket_ == nullptr)
        throw Crab_error{"HTTP_socket::send: Not Connected."};

    namespace http = boost::beast::http;
    auto req = http::request<http::string_body>{http::verb::get, resource, 11};
    req.set(http::field::host, host_);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    auto count = 0;
    auto ec    = boost::system::error_code{};
    auto resp  = http::response<http::dynamic_body>{};
    while (true) {
        http::write(*socket_, req);
        http::read(*socket_, buffer_, resp, ec);
        ++count;
        if (!ec.failed())
            break;
        if (count == 5)
            throw Crab_error{"HTTPS_socket::get: Failed with 5 attempts."};
        this->reconnect();
    };

    return {resp.result_int(),
            boost::beast::buffers_to_string(resp.body().data())};
}

}  // namespace crab
