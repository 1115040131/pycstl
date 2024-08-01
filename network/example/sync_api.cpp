#include "network/example/sync_api.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <fmt/ostream.h>

namespace network {

namespace asio = boost::asio;

#pragma region socket 的监听和连接

int ClientEndPoint() {
    // Step 1. Assume that the client application has already obtained the IP-address and the protocol port number.
    std::string raw_ip_address = "127.4.8.1";
    unsigned short port_num = 3333;

    // Step 2. Using IP protocol version independent address representation.
    boost::system::error_code error_code;
    asio::ip::address ip_address = asio::ip::address::from_string(raw_ip_address, error_code);
    if (error_code.value() != 0) [[unlikely]] {
        fmt::println("Failed to parse the IP address {}. Error code = {}, Message: {}", raw_ip_address,
                     error_code.value(), error_code.message());
        return error_code.value();
    }

    // Step 3.
    asio::ip::tcp::endpoint endpoint(ip_address, port_num);

    // Step 4. The endpoint is ready and can be used to specify a particular server in the network the client wants
    // to communicate with.

    return 0;
}

int ServerEndPoint() {
    // Step 1. Here we assume that the server application has already obtained the protocol port number.
    unsigned short port_num = 3333;

    // Step 2. Create special object of asio::ip::address class that specifies all IP-addresses available on the
    // host. Note that here we assume that server works over IPv6 protocol.
    asio::ip::address ip_address = asio::ip::address_v4::any();
    // asio::ip::address ip_address = asio::ip::address_v6::any();

    // Step3.
    asio::ip::tcp::endpoint endpoint(ip_address, port_num);

    // Step 4. The endpoint is created and can be used to specify the IP addresses and a port number on which the
    // server application wants to listen for incoming connections.

    return 0;
}

int CreateTcpSocket() {
    // Step 1. An instance of 'io_service' class is required by socket constructor.
    asio::io_context io_context;

    // Step 2. Creating an object of 'tcp' class representing a TCP protocol with IPv4 as underlying protocol.
    asio::ip::tcp protocol = asio::ip::tcp::v4();

    // Step 3. Instantiating an active TCP socket object.
    asio::ip::tcp::socket sock(io_context);

    // 高版本的 asio 库不需要手动 open, 在连接时会自动执行

    // Step 4. Opening the socket.
    boost::system::error_code error_code;
    sock.open(protocol, error_code);
    if (error_code.value() != 0) [[unlikely]] {
        fmt::println("Failed to open the socket! Error code = {}, Message: {}", error_code.value(),
                     error_code.message());
        return error_code.value();
    }

    return 0;
}

int CreateAcceptorSocket() {
    unsigned short port_num = 3333;
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_num));

    return 0;
}

int create_acceptor_socket() {
    // Step 1. An instance of 'io_service' class is required by socket constructor.
    asio::io_context io_context;

    // Step 2. Creating an object of 'tcp' class representing a TCP protocol with IPv6 as underlying protocol.
    asio::ip::tcp protocol = asio::ip::tcp::v4();

    // Step 3. Instantiating an acceptor socket object.
    asio::ip::tcp::acceptor acceptor(io_context);

    // Step 4. Opening the acceptor socket.
    boost::system::error_code error_code;
    acceptor.open(protocol, error_code);
    if (error_code.value() != 0) {
        fmt::println("Failed to open the acceptor socket! Error code = {}. Message: {}", error_code.value(),
                     error_code.message());
        return error_code.value();
    }

    return 0;
}

int bind_acceptor_socket() {
    // Step 1. Here we assume that the server application has already obtained the protocol port number.
    unsigned short port_num = 3333;

    // Step 2. Creating an endpoint.
    asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::any(), port_num);

    // Step 3. Creating and opening an acceptor socket.
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(io_context, endpoint.protocol());

    // Step 4. Binding the acceptor socket.
    boost::system::error_code error_code;
    acceptor.bind(endpoint, error_code);
    if (error_code.value() != 0) {
        fmt::println(std::cerr, "Failed to bind the acceptor socket! Error code = {}. Message: {}",
                     error_code.value(), error_code.message());
        return error_code.value();
    }

    return 0;
}

int ConnectToEnd() {
    // Step 1. Assume that the client application has already obtained the IP address and protocol port number of
    // the target server.
    std::string raw_ip_address = "192.168.1.124";
    unsigned short port_num = 3333;

    try {
        // Step 2. Creating an endpoint designating a target server application.
        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(raw_ip_address), port_num);

        // Step 3. Creating and opening a socket.
        asio::io_context io_context;
        asio::ip::tcp::socket sock(io_context, endpoint.protocol());

        // Step 4. Connecting a socket.
        sock.connect(endpoint);
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

int DnsConnectToEnd() {
    std::string host = "llfc.club";
    std::string port_num = "3333";
    asio::io_context io_context;
    asio::ip::tcp::resolver::query query(host, port_num, asio::ip::tcp::resolver::query::numeric_service);  // 查询
    asio::ip::tcp::resolver resolver(io_context);  // 域名解析器

    try {
        asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
        asio::ip::tcp::socket sock(io_context);
        asio::connect(sock, iter);
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

int AcceptNewConnection() {
    constexpr int kBacklogSize = 30;

    // Step 1. Here we assume that the server application has already obtained the protocol port number.
    unsigned short port_num = 3333;

    // Step 2. Creating a server endpoint.
    asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::any(), port_num);

    try {
        // Step 3. Instantiating and opening an acceptor socket.
        asio::io_context io_context;
        asio::ip::tcp::acceptor acceptor(io_context, endpoint.protocol());

        // Step 4. Binding the acceptor socket to the server endpint.
        acceptor.bind(endpoint);

        // Step 5. Starting to listen for incoming connection requests.
        acceptor.listen(kBacklogSize);

        // Step 6. Creating an active socket.
        asio::ip::tcp::socket sock(io_context);

        // Step 7. Processing the next connection request and connecting the active socket to the client.
        acceptor.accept(sock);
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

#pragma endregion

#pragma region buffer

void UseConstBuffer() {
    std::string buf = "hello world";
    asio::const_buffer asio_buf(buf.c_str(), buf.size());
    std::vector<asio::const_buffer> buffer_sequence;
    buffer_sequence.push_back(asio_buf);
}

void UseBufferStr() { [[maybe_unused]] asio::const_buffers_1 output_buf = asio::buffer("hello world"); }

void UseBufferArray() {
    constexpr std::size_t kBufSizeBytes = 20;
    std::unique_ptr<char[]> buf(new char[kBufSizeBytes]);
    [[maybe_unused]] asio::mutable_buffers_1 input_buf =
        asio::buffer(static_cast<void*>(buf.get()), kBufSizeBytes);
}

void UseStreamBuffer() {
    asio::streambuf buf;
    std::ostream output(&buf);

    // Writing the message to the stream-based buffer.
    output << "Message1\nMessage2";

    std::istream input(&buf);

    std::string message1;

    input >> message1;
}

#pragma endregion

#pragma region 同步读写

/// @brief 同步写 write_some
void WriteToSocket(asio::ip::tcp::socket& sock) {
    std::string buf = "Hello World";
    std::size_t total_bytes_written = 0;

    // 循环发送
    while (total_bytes_written != buf.length()) {
        total_bytes_written +=
            sock.write_some(asio::buffer(buf.c_str() + total_bytes_written, buf.length() - total_bytes_written));
    }
}

int SendDataByWriteSome() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context io_context;
        asio::ip::tcp::socket sock(io_context, endpoint.protocol());
        sock.connect(endpoint);
        WriteToSocket(sock);
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

/// @brief 同步写 send
int SendDataBySend() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context io_context;
        asio::ip::tcp::socket sock(io_context, endpoint.protocol());
        sock.connect(endpoint);

        std::string buf = "Hello World";

        // 一次性发送
        // send_length = 0: 对端关闭
        //             > 0: 一定等于 buf.length(), 否则阻塞
        //             < 0: 错误
        auto send_length = sock.send(asio::buffer(buf.c_str(), buf.length()));
        if (send_length <= 0) {
            fmt::println("Send failed!");
            return 1;
        }
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

/// @brief 同步写 write
int SendDataByWrite() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context io_context;
        asio::ip::tcp::socket sock(io_context, endpoint.protocol());
        sock.connect(endpoint);

        std::string buf = "Hello World";

        // 一次性发送
        // send_length = 0: 对端关闭
        //             > 0: 一定等于 buf.length(), 否则阻塞
        //             < 0: 错误
        auto send_length = asio::write(sock, asio::buffer(buf.c_str(), buf.length()));
        if (send_length <= 0) {
            fmt::println("Write failed!");
            return 1;
        }
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

/// @brief 同步读 read_some
std::string ReadFromSocket(asio::ip::tcp::socket& sock) {
    constexpr unsigned char kMessageSize = 7;
    char buf[kMessageSize];
    std::size_t total_bytes_read = 0;

    // 循环读
    while (total_bytes_read != kMessageSize) {
        total_bytes_read += sock.read_some(asio::buffer(buf + total_bytes_read, kMessageSize - total_bytes_read));
    }

    return std::string(buf, total_bytes_read);
}

int ReadDataByReadSome() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context io_context;
        asio::ip::tcp::socket sock(io_context, endpoint.protocol());
        sock.connect(endpoint);
        ReadFromSocket(sock);
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

/// @brief 同步读 receive
int ReadDataByReceive() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context io_context;
        asio::ip::tcp::socket sock(io_context, endpoint.protocol());
        sock.connect(endpoint);

        constexpr unsigned char kMessageSize = 7;
        char buffer_receive[kMessageSize];
        int receive_length = sock.receive(asio::buffer(buffer_receive, kMessageSize));
        if (receive_length <= 0) {
            fmt::println("Receive failed!");
            return 1;
        }
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

/// @brief 同步读 read
int ReadDataByRead() {
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;

    try {
        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context io_context;
        asio::ip::tcp::socket sock(io_context, endpoint.protocol());
        sock.connect(endpoint);

        constexpr unsigned char kMessageSize = 7;
        char buffer_receive[kMessageSize];
        int receive_length = asio::read(sock, asio::buffer(buffer_receive, kMessageSize));
        if (receive_length <= 0) {
            fmt::println("Receive failed!");
            return 1;
        }
    } catch (const boost::system::system_error& e) {
        fmt::println(std::cerr, "Error occured! Error code = {}. Message: {}", e.code().value(), e.what());

        return e.code().value();
    }

    return 0;
}

/// @brief 读取直到指定字符
std::string ReadDataByUtil(asio::ip::tcp::socket& sock) {
    asio::streambuf buf;

    asio::read_until(sock, buf, '\n');

    std::string message;

    std::istream input(&buf);
    std::getline(input, message);
    return message;
}

#pragma endregion

}  // namespace network
