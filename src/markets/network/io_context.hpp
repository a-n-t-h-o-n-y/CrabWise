#ifndef CRAB_MARKETS_NETWORK_IO_CONTEXT_HPP
#define CRAB_MARKETS_NETWORK_IO_CONTEXT_HPP
#include <boost/asio/io_context.hpp>

namespace crab {

/// Return the global io_context object.
inline auto io_context() -> boost::asio::io_context&
{
    static auto ctx = boost::asio::io_context{};
    return ctx;
}

}  // namespace crab
#endif  // CRAB_MARKETS_NETWORK_IO_CONTEXT_HPP
