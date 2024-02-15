# HTTP 包头信息

一个标准的HTTP报文头通常由请求头和响应头两部分组成。

## HTTP 请求头

HTTP请求头包括以下字段：

- **Request-line**：包含用于描述请求类型、要访问的资源以及所使用的HTTP版本的信息。
- **Host**：指定被请求资源的主机名或IP地址和端口号。
- **Accept**：指定客户端能够接收的媒体类型列表，用逗号分隔，例如 text/plain, text/html。
- **User-Agent**：客户端使用的浏览器类型和版本号，供服务器统计用户代理信息。
- **Cookie**：如果请求中包含cookie信息，则通过这个字段将cookie信息发送给Web服务器。
- **Connection**：表示是否需要持久连接（keep-alive）。

比如下面就是一个实际应用

```
GET /index.html HTTP/1.1
Host: www.example.com
Accept: text/html, application/xhtml+xml, */*
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:123.0) Gecko/20100101 Firefox/123.0
Cookie: sessionid=abcdefg1234567
Connection: keep-alive
```

## HTTP 响应头

HTTP响应头包括以下字段：

- **Status-line**：包含协议版本、状态码和状态消息。
- **Content-Type**：响应体的MIME类型。
- **Content-Length**：响应体的字节数。
- **Set-Cookie**：服务器向客户端发送cookie信息时使用该字段。
- **Server**：服务器类型和版本号。
- **Connection**：表示是否需要保持长连接（keep-alive）。

在实际的HTTP报文头中，还可以包含其他可选字段。\
如下是一个http响应头的示例

```
HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Content-Length: 1024
Set-Cookie: sessionid=abcdefg1234567; HttpOnly; Path=/
Server: Apache/2.2.32 (Unix) mod_ssl/2.2.32 OpenSSL/1.0.1e-fips mod_bwlimited/1.4
Connection: keep-alive
```