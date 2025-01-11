#include <unp.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fmt/core.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>


int main(int argc, char* argv[])
{
    auto socket =unp::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = ::htons(unp::private_port::Daytime);
    address.sin_addr.s_addr = ::htonl(INADDR_ANY);

    unp::bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));

    unp::listen(socket, 5);

    char buffer[unp::MaxLine];
    int connected_fd;
    while (true) {
        connected_fd = unp::accept(socket, nullptr, nullptr);

        auto ticks = std::time(nullptr);
        snprintf(buffer, unp::MaxLine, "%.24s\r\n", std::ctime(&ticks));
        unp::write(connected_fd, buffer, std::strlen(buffer));

        unp::close(connected_fd);
    }

    return EXIT_SUCCESS;
}