#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace pyc {
namespace chat {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

}  // namespace chat
}  // namespace pyc