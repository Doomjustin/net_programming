#include <unp.h>
#include <fmt/core.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstdio>


void str_cli(FILE* fd, int sockfd);

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

    str_cli(stdin, socket);

    return EXIT_SUCCESS;
}


void str_cli(FILE* fp, int sockfd)
{
    fd_set read_sets{};
    FD_ZERO(&read_sets);
    bool stdin_eof = false;
    char buffer[unp::MaxLine];

    while (true) {
        if (!stdin_eof)
            FD_SET(fileno(fp), &read_sets);

        FD_SET(sockfd, &read_sets);

        auto max_fds = std::max(fileno(fp), sockfd) + 1;
        unp::select(max_fds, &read_sets, nullptr, nullptr, nullptr);

        if (FD_ISSET(sockfd, &read_sets)) {
            auto read_bytes = unp::read(sockfd, buffer, unp::MaxLine);
            if (read_bytes == 0) {
                if (stdin_eof) return;
                else unp::system_error("str_cli: server terminated prematurely");
            }

            unp::write(fileno(fp), buffer, read_bytes);
        }

        if (FD_ISSET(fileno(fp), &read_sets)) {
            auto read_bytes = unp::read(fileno(fp), buffer, unp::MaxLine);
            if (read_bytes == 0) {
                stdin_eof = true;
                unp::shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &read_sets);
                continue;
            }

            unp::write(sockfd, buffer, read_bytes);
        }
    }

    unp::close(sockfd);
}
