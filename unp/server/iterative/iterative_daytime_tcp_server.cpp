#include <unp.h>
#include <fmt/core.h>
#include <fmt/chrono.h>

#include <ctime>
#include <cstdlib>


int main(int argc, char* argv[])
{
    unp::Socket<unp::IPv4, unp::TCP> socket{};

    unp::Endpoint target{ unp::private_port::Daytime };
    socket.bind(target);
    socket.listen();

    while (true) {
        auto accept_socket = socket.accept();
        fmt::println("accept socket");

        auto now = std::time(nullptr);
        auto now_str = fmt::format("{:%c}\r\n", fmt::localtime(now));

        accept_socket.write(now_str);
        fmt::println("write time[{}].\nclosed", now_str);
    }

    return EXIT_SUCCESS;
}