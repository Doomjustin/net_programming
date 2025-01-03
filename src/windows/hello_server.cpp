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
    if (argc != 2) {
        fmt::println("Usage: {} <port>", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    WSADATA wsa_data;
    if (::WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        error_handling("WSAStartup() error");

    auto server_sock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET)
        error_handling("socket() error");

    SOCKADDR_IN server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = ::htonl(ADDR_ANY);
    server_address.sin_port = ::htons(std::stoi(argv[1]));

    if (::bind(server_sock, reinterpret_cast<SOCKADDR*>(&server_address), sizeof(server_address)) == SOCKET_ERROR)
        error_handling("bind() error");

    constexpr int ListenQ = 5;
    if (::listen(server_sock, ListenQ) == SOCKET_ERROR)
        error_handling("listen() error");

    SOCKADDR_IN client_address{};
    int client_address_size = sizeof(client_address);
    auto client_socket = ::accept(server_sock, reinterpret_cast<SOCKADDR*>(&client_address), &client_address_size);
    if (client_socket == INVALID_SOCKET)
        error_handling("accept() error");

    std::string message = "Hello, World!";
    ::send(client_socket, message.c_str(), message.size(), 0);
    
    ::closesocket(client_socket);
    ::closesocket(server_sock);
    ::WSACleanup();
    return EXIT_SUCCESS;
}

void error_handling(std::string_view message)
{
    fmt::println(stderr, "{}", message);
    std::exit(EXIT_FAILURE);
}