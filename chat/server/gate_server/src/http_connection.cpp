#include "gate_server/http_connection.h"

#include <iostream>

#include <fmt/ostream.h>

#include "gate_server/logic_system.h"

namespace pyc {
namespace chat {

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket)) {}

void HttpConnection::Start() {
    http::async_read(socket_, buffer_, request_,
                     [self = shared_from_this()](const beast::error_code& ec, std::size_t) {
                         try {
                             if (ec) {
                                 fmt::println(std::cerr, "{}", ec.what());
                                 return;
                             }

                             self->HandleRequest();
                             self->CheckDeadline();
                         } catch (const std::exception& e) {
                             fmt::print(std::cerr, "{}\n", e.what());
                         }
                     });
}

void HttpConnection::HandleRequest() {
    response_.version(request_.version());
    response_.keep_alive(false);
    if (request_.method() == http::verb::get) {
        ParseParams();
        bool success = LogicSystem::GetInstance().HandleGet(get_url_, shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "Url not found\r\n";
            WriteResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    } else if (request_.method() == http::verb::post) {
        bool success = LogicSystem::GetInstance().HandlePost(request_.target(), shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "Url not found\r\n";
            WriteResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }
}

void HttpConnection::WriteResponse() {
    response_.content_length(response_.body().size());
    http::async_write(socket_, response_, [self = shared_from_this()](beast::error_code ec, std::size_t) {
        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        self->dealine_.cancel();
    });
}

void HttpConnection::CheckDeadline() {
    dealine_.async_wait([self = shared_from_this()](const beast::error_code& ec) {
        if (!ec) {
            self->socket_.close();
        }
    });
}

constexpr unsigned char ToHex(unsigned char x) { return x > 9 ? x + 55 : x + 48; }

constexpr unsigned char FromHex(unsigned char x) {
    if (x >= 'A' && x <= 'Z') {
        return x - 'A' + 10;
    } else if (x >= 'a' && x <= 'z') {
        return x - 'a' + 10;
    } else if (x >= '0' && x <= '9') {
        return x - '0';
    }
    return 0;
}

constexpr std::string UrlEncode(std::string_view str) {
    std::string result;
    for (char c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else if (c == ' ') {  // 空字符
            result += '+';
        } else {
            // 高 4 位和低 4 位分别转换为 16 进制
            result += '%';
            result += ToHex(c >> 4);
            result += ToHex(c & 0x0F);
        }
    }
    return result;
}

// 解析 url 请求
constexpr std::string UrlDecode(std::string_view str) {
    std::string result;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '+') {
            result += ' ';
        } else if (str[i] == '%') {
            unsigned char high = FromHex(str[++i]);
            unsigned char low = FromHex(str[++i]);
            result += high * 16 + low;
        } else {
            result += str[i];
        }
    }
    return result;
}

void HttpConnection::ParseParams() {
    auto uri = request_.target();

    // 找到参数开始的位置 ?
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos) {
        get_url_ = uri;
        return;
    }

    get_url_ = uri.substr(0, query_pos);
    auto query_string = uri.substr(query_pos + 1);

    size_t begin_pos = 0;
    while (true) {
        // 找到每组参数的分隔符 =
        size_t eq_pos = query_string.find('=', begin_pos);
        if (eq_pos == std::string::npos) {
            break;
        }
        // 找到每组参数的结束符 &
        size_t end_pos = query_string.find('&', eq_pos);
        if (end_pos == std::string::npos) {  // 最后一个参数
            end_pos = query_string.size();
        }
        auto key = query_string.substr(begin_pos, eq_pos - begin_pos);
        auto value = query_string.substr(eq_pos + 1, end_pos - eq_pos - 1);
        if (!key.empty() && !value.empty()) {
            get_params_[UrlDecode(key)] = UrlDecode(value);
        }

        // 所有参数解析完毕
        if (end_pos == query_string.size()) {
            break;
        }
        begin_pos = end_pos + 1;
    }
}

}  // namespace chat
}  // namespace pyc
