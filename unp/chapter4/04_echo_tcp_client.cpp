#include <iostream>
#include <unp.h>
#include <fmt/core.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>


void str_cli(int sockfd);

int main(int argc, char* argv[])
{
    if (argc != 2)
        unp::system_error("usage: daytime_tcp_client <IP address>");

    auto socket = unp::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = ::htons(unp::private_port::Echo);

    unp::inet_pton(AF_INET, argv[1], &address.sin_addr);

    unp::connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));

    str_cli(socket);

    return EXIT_SUCCESS;
}


void str_cli(int sockfd)
{
    std::string line{};
    while (std::cin) {
        std::cin >> line;
        line.push_back('\n');
        unp::write(sockfd, line.data(), line.size());

        auto received_msg = unp::readline(sockfd);
        fmt::println("{}", received_msg);
    }

    unp::close(sockfd);
}
