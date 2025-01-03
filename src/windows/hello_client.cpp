#include <fmt/core.h>

#include <WinSock2.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <string_view>


void error_handling(std::string_view message);

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fmt::println("Usage: {} <IP> <port>", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    WSADATA wsa_data;
    if (::WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        error_handling("WSAStartup() error");

    auto socket = ::socket(PF_INET, SOCK_STREAM, 0);
    if (socket == INVALID_SOCKET)
        error_handling("socket() error");

    SOCKADDR_IN server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = ::htons(std::stoi(argv[2]));
    if (::inet_pton(AF_INET, argv[1], &server_address.sin_addr) == SOCKET_ERROR)
        error_handling("inet_pton() error");

    
    if (::connect(socket, reinterpret_cast<SOCKADDR*>(&server_address), sizeof(server_address)) == SOCKET_ERROR)
        error_handling("connect() error");
        
    char buffer[30];
    auto received_bytes = ::recv(socket, buffer, 30, 0);
    if (received_bytes == -1)
        error_handling("recv() error");
    buffer[received_bytes] = '\0';
    fmt::println("Message from server: {}", buffer);

    ::closesocket(socket);
    ::WSACleanup();
    return EXIT_SUCCESS;
}

void error_handling(std::string_view message)
{
    fmt::println(stderr, "{}", message);
    std::exit(EXIT_FAILURE);
}