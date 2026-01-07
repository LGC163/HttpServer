#include <iostream>
#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class HttpsClient {
public:
    HttpsClient() {
        // 初始化 OpenSSL
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();

        ctx_ = SSL_CTX_new(TLS_client_method());
        if (!ctx_) {
            throw std::runtime_error("创建 SSL_CTX 失败");
        }
    }

    ~HttpsClient() {
        if (ssl_) SSL_free(ssl_);
        if (ctx_) SSL_CTX_free(ctx_);
        if (sock_ > 0) close(sock_);
    }

    void connect(const std::string& host, int port) {
        // 创建套接字
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) {
            throw std::runtime_error("创建套接字失败");
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(host.c_str());

        // 连接服务器
        if (::connect(sock_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            throw std::runtime_error("连接服务器失败");
        }

        // 创建 SSL 对象
        ssl_ = SSL_new(ctx_);
        if (!ssl_) {
            throw std::runtime_error("创建 SSL 对象失败");
        }

        SSL_set_fd(ssl_, sock_);
        if (SSL_connet(ssl_) <= 0) {
            throw std::runtime_error("SSL 连接失败");
        }

        std::cout << "连接使用的加密算法: " << SSL_get_cipher(ssl_) << std::endl;
    }

    // 发送 HTTP 请求
    void sendRequest(const std::string& path) {
        std::cout << "发送请求出去sedGetRequest" << std::endl;
        std::string request =
            "GET " + path + " HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "Connection: close\r\n\r\n";

        if (SSL_write(ssl_, request.c_str(), request.size()) <= 0) {
            throw std::runtime_error("发送请求失败");
        }

        char buffer[4096];
        int bytes;
        while ((bytes = SSL_read(ssl_, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes] = 0;
            std::cout << buffer;
        }
    }

private:
    SSL_CTX* ctx_ = nullptr;
    SSL* ssl_ = nullptr;
    int sock_ = -1;
};

int main() {
    try {
        HttpsClient client;
        client.connect("127.0.0.1", 443);
        std::cout << "连接成功connected" << std::endl;
        client.sendRequest("/");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
}