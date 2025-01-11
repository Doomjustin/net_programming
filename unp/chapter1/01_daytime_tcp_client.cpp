#include <unp.h>

#include <cstdlib>
#include <fmt/core.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>


int main(int argc, char* argv[])
{
    if (argc != 2)
        unp::system_error("usage: daytime_tcp_client <IP address>");

    auto socket = unp::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = ::htons(unp::private_port::Daytime);

    unp::inet_pton(AF_INET, argv[1], &address.sin_addr);

    unp::connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));

    char receive_line[unp::MaxLine];
    ssize_t n = 0;
    while ((n = unp::read(socket, receive_line, unp::MaxLine)) > 0) {
        receive_line[n] = 0;
        fmt::print("{}", receive_line);
    }

    return EXIT_SUCCESS;
}