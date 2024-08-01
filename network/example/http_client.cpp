#include <iostream>
#include <regex>
#include <string>

#include <boost/asio.hpp>
#include <fmt/format.h>

namespace asio = boost::asio;
using asio::ip::tcp;

class HttpClient {
public:
    HttpClient(asio::io_service& io_service, const std::string& server, const std::string& path)
        : resolver_(io_service), socket_(io_service) {
        if (!IsValidIpAddress(server)) {
            throw std::invalid_argument(fmt::format("Invalid IP address: {}", server));
        }

        std::ostream request_stream(&request_);
        request_stream << fmt::format(
            "GET {} HTTP/1.0\r\n"
            "Host: {}\r\n"
            "Accept: */*\r\n"
            "Connection: close\r\n\r\n",
            path, server);

        std::size_t pos = server.find(":");
        std::string_view ip = server.substr(0, pos);
        std::string_view port = server.substr(pos + 1);

        resolver_.async_resolve(
            ip, port,
            [this](const boost::system::error_code& error_code, const tcp::resolver::results_type& endpoints) {
                HandleResolve(error_code, endpoints);
            });
    }

private:
    bool IsValidIpAddress(const std::string& path) {
        // 使用正则表达式匹配IPv4地址和端口号格式（例如：192.168.1.1:8080）
        std::regex path_format("^(\\d{1,3}(\\.\\d{1,3}){3}):(\\d{1,5})$");
        return std::regex_match(path, path_format);
    }

    void HandleResolve(const boost::system::error_code& error_code, const tcp::resolver::results_type& endpoints) {
        if (error_code) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
            return;
        }

        asio::async_connect(socket_, endpoints,
                            [this](const boost::system::error_code& error_code, const tcp::endpoint&) {
                                HandleConnect(error_code);
                            });
    }

    void HandleConnect(const boost::system::error_code& error_code) {
        if (error_code) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
            return;
        }

        asio::async_write(socket_, request_, [this](const boost::system::error_code& error_code, std::size_t) {
            HandleWriteRequest(error_code);
        });
    }

    void HandleWriteRequest(const boost::system::error_code& error_code) {
        if (error_code) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
            return;
        }

        asio::async_read_until(socket_, response_, "\r\n",
                               [this](const boost::system::error_code& error_code, std::size_t) {
                                   HandleReadStatusLine(error_code);
                               });
    }

    void HandleReadStatusLine(const boost::system::error_code& error_code) {
        if (error_code) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
            return;
        }

        // 检查回复状态
        std::istream response_stream(&response_);
        std::string http_version;
        response_stream >> http_version;
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            fmt::println("[{}]: Invalid response", __func__);
        }

        unsigned int status_code;
        response_stream >> http_version >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (status_code != 200) {
            fmt::println("[{}]: Response returned with status code: {}, status message: {}", __func__, status_code,
                         status_message);
        }

        asio::async_read_until(
            socket_, response_, "\r\n\r\n",
            [this](const boost::system::error_code& error_code, std::size_t) { HandleReadHeaders(error_code); });
    }

    void HandleReadHeaders(const boost::system::error_code& error_code) {
        if (error_code) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
            return;
        }

        // 处理回复头
        std::istream response_stream(&response_);
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            fmt::println("{}", header);
        }
        fmt::print("\n");

        // 输出当前读到的所有内容
        if (response_.size() > 0) {
            // fmt::print("{}", &response_);
            std::cout << &response_;
        }

        // 读取直到 EOF
        asio::async_read(
            socket_, response_, asio::transfer_at_least(1),
            [this](const boost::system::error_code& error_code, std::size_t) { HandleReadContent(error_code); });
    }

    void HandleReadContent(const boost::system::error_code& error_code) {
        if (error_code && error_code != asio::error::eof) {
            fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
            return;
        }

        // 输出当前读到的所有内容
        std::cout << &response_;

        // 读取剩余数据直到 EOF
        asio::async_read(
            socket_, response_, asio::transfer_at_least(1),
            [this](const boost::system::error_code& error_code, std::size_t) { HandleReadContent(error_code); });
    }

private:
    tcp::resolver resolver_;
    tcp::socket socket_;
    asio::streambuf request_{};
    asio::streambuf response_{};
};

int main() {
    try {
        asio::io_context io_context;
        HttpClient client(io_context, "127.0.0.1:8080", "/");
        io_context.run();
        getchar();
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    return 0;
}