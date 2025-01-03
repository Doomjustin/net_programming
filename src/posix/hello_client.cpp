#include "fmt/base.h"
#include <fmt/core.h>

#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>


void error_handling(std::string_view message);

int main(int argc, char* argv[])
{
    constexpr int InvalidSocket = -1;
    constexpr int InvalidResult = -1;
    constexpr int ListenQ = 5;

    if (argc != 3) {
        fmt::println("Usage: {} <IP> <port>", argv[0]);
        return EXIT_FAILURE;
    }

    auto sock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (sock == InvalidSocket)
        error_handling("socket() error");

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = ::htons(std::stoi(argv[2]));
    auto res = ::inet_pton(AF_INET, argv[1], &server_address.sin_addr);
    if (res == InvalidResult)
        error_handling("inet_pton() error");

    if (::connect(sock, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == InvalidResult)
        error_handling("connect() error");
    
    char buffer[30];
    auto read_bytes = ::read(sock, buffer, 30);
    if (read_bytes == InvalidResult)
        error_handling("read() error");
    
    buffer[read_bytes] = '\0';

    fmt::println("Message from server: {}", buffer);
    ::close(sock);

    return EXIT_SUCCESS;
}

void error_handling(std::string_view message)
{
    fmt::println(stderr, "{}", message);
    std::exit(EXIT_FAILURE);
}
