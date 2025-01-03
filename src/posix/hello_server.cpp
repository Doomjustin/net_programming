#include <fmt/core.h>

#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>


void error_handling(std::string_view message);

int main(int argc, char* argv[])
{
    constexpr int InvalidSocket = -1;
    constexpr int InvalidResult = -1;
    constexpr int ListenQ = 5;

    if (argc != 2) {
        fmt::println("Usage: {} <port>", argv[0]);
        return EXIT_FAILURE;
    }

    auto server_sock = ::socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == InvalidSocket)
        error_handling("socket() error");

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = ::htonl(INADDR_ANY);
    server_address.sin_port = ::htons(std::stoi(argv[1]));

    if (::bind(server_sock, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == InvalidResult)
        error_handling("bind() error");

    if (::listen(server_sock, ListenQ) == InvalidResult)
        error_handling("listen() error");


    sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);

    auto client_sock = ::accept(server_sock, reinterpret_cast<sockaddr*>(&client_address), &client_address_size);
    if (client_sock == InvalidSocket)
        error_handling("accept() error");
    
    const std::string message = "Hello, world!";
    ::write(client_sock, message.c_str(), message.size());
    ::close(client_sock);
    ::close(server_sock);

    return EXIT_SUCCESS;
}

void error_handling(std::string_view message)
{
    fmt::println(stderr, "{}", message);
    std::exit(EXIT_FAILURE);
}
