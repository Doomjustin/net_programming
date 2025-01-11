#include <fmt/core.h>
#include <unp.h>

#include <sys/wait.h>
#include <cstdlib>

void echo(unp::Socket<unp::IPv4, unp::TCP> socket);


int main(int argc, char* argv[])
{
    signal(SIGCHLD, unp::sig_child);

    unp::Socket<unp::IPv4, unp::TCP> socket{};

    unp::Endpoint target{ unp::private_port::Echo };
    socket.bind(target);
    socket.listen();

    pid_t n;
    while (true) {
        auto accept_socket = socket.accept();

        if ((n = unp::fork()) == 0) {
            // 反正程序结束的时候会自动close，所以不手动close也行
            // socket.close();
            echo(std::move(accept_socket));
            break;
        }
    }

    return EXIT_SUCCESS;
}


void echo(unp::Socket<unp::IPv4, unp::TCP> socket)
{
    fmt::println("connected form {}", socket.peer().address());

    while (socket) {
        auto received_msg = socket.read(unp::MaxLine);
        if (received_msg) {
            fmt::println("{}", *received_msg);
            socket.write(*received_msg);
        }
        else {
            fmt::println("client closed");
            break;
        }
    }
}
