#include <chrono>
#include <memory>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

using namespace std::literals::chrono_literals;

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;

namespace network {

std::size_t RequestCount() {
    static std::size_t count = 0;
    return ++count;
}

std::int64_t Now() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
    static constexpr std::size_t kBufferSize = 8 * 1024;

public:
    HttpConnection(tcp::socket&& socket) : socket_(std::move(socket)) {}

    void Start() {
        auto shared_this = shared_from_this();
        http::async_read(socket_, buffer_, request_,
                         [shared_this](const boost::beast::error_code& error_code, std::size_t) {
                             shared_this->ProcessRequest(error_code);
                         });

        deadline_.async_wait([shared_this](const boost::system::error_code& error_code) {
            shared_this->CheckDeadline(error_code);
        });
    }

private:
    void ProcessRequest(const boost::system::error_code& error_code) {
        if (error_code) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
            return;
        }

        response_.version(request_.version());
        response_.keep_alive(false);
        switch (request_.method()) {
            case http::verb::get:
                response_.result(http::status::ok);
                response_.set(http::field::server, "Beast");
                CreateResponse();
                break;

            case http::verb::post:
                response_.result(http::status::ok);
                response_.set(http::field::server, "Beast");
                CreatePostResponse();
                break;

            default:
                response_.result(http::status::bad_request);
                response_.set(http::field::content_type, "text/plain");
                auto method_string = request_.method_string();
                beast::ostream(response_.body()) << fmt::format(
                    "Invalid request-method \"{}\"", std::string_view(method_string.data(), method_string.size()));
                break;
        }

        response_.content_length(response_.body().size());
        http::async_write(socket_, response_,
                          [shared_this = shared_from_this()](const boost::beast::error_code&, std::size_t) {
                              // 服务器主动关闭会出现大量的 establish wait, 因此只关闭发送端等待客服端发送关闭
                              shared_this->socket_.shutdown(tcp::socket::shutdown_send);
                              shared_this->deadline_.cancel();
                          });
    }

    void CreateResponse() {
        if (request_.target() == "/count") {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body()) << R"(<html>
<head><title>Request count</title></head>
<body>
<h1>Request count</h1>
<p>There have been )" << RequestCount() << R"( requests so far.</p>
</body>
</html>
)";
        } else if (request_.target() == "/time") {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body()) << R"(<html>
<head><title>Request count</title></head>
<body>
<h1>Request count</h1>
<p>There have been )" << Now() << R"( requests so far.</p>
</body>
</html>
)";
        } else {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "File not found\r\n";
        }
    }

    void CreatePostResponse() {
        if (request_.target() == "/email") {
            auto body_str = beast::buffers_to_string(request_.body().data());
            fmt::println("[{}]: Receive body is {}", __func__, body_str);
            response_.set(http::field::content_type, "text/json");
            try {
                auto reader = nlohmann::json::parse(body_str);
                std::string email = reader.at("email");
                fmt::println("[{}]: email is {}", __func__, email);

                nlohmann::json root;
                root["error"] = 0;
                root["email"] = email;
                root["msg"] = "Receive email post success";
                beast::ostream(response_.body()) << root.dump();
            } catch (const std::exception& e) {
                fmt::println(stderr, "[{}]: Json exception: {}", __func__, e.what());

                nlohmann::json root;
                root["error"] = 1001;
                beast::ostream(response_.body()) << root.dump();
            }

        } else {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "File not found\r\n";
        }
    }

    void CheckDeadline(const boost::system::error_code& error_code) {
        if (error_code) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
        }

        socket_.close();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{kBufferSize};
    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;

    asio::steady_timer deadline_{socket_.get_executor(), 60s};
};

void HttpServer(tcp::acceptor& acceptor) {
    auto socket = std::make_shared<tcp::socket>(acceptor.get_executor());
    acceptor.async_accept(*socket, [socket, &acceptor](const boost::system::error_code& error_code) {
        if (!error_code) {
            std::make_shared<HttpConnection>(std::move(*socket))->Start();
        } else {
            fmt::println("[async_accept]: Error code: {}. Message: {}", error_code.value(), error_code.message());
        }
        HttpServer(acceptor);
    });
}

}  // namespace network

int main() {
    try {
        const auto address = asio::ip::make_address("127.0.0.1");
        unsigned short port = 8080u;
        asio::io_context io_context{1};
        tcp::acceptor acceptor{io_context, {address, port}};
        network::HttpServer(acceptor);
        io_context.run();
    } catch (const std::exception& e) {
        fmt::println(stderr, "[{}]: Exception: {}", __func__, e.what());
    }

    return 0;
}