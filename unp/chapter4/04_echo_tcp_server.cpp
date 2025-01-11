#include <cerrno>
#include <csignal>
#include <unp.h>
#include <fmt/core.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

void str_echo(int connected_fd);
void sig_child(int signo);


int main(int argc, char* argv[])
{
    signal(SIGCHLD, sig_child);

    auto socket =unp::socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = ::htons(unp::private_port::Echo);
    address.sin_addr.s_addr = ::htonl(INADDR_ANY);

    unp::bind(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));

    unp::listen(socket, 5);

    char buffer[unp::MaxLine];
    int connected_fd;
    int child_fd;
    while (true) {
        connected_fd = unp::accept(socket, nullptr, nullptr);

        if ((child_fd = unp::fork()) == 0) {
            unp::close(socket);
            str_echo(connected_fd);
            break;
        }

        unp::close(connected_fd);
    }

    return EXIT_SUCCESS;
}


void str_echo(int connected_fd)
{
    fmt::println("connected {}", connected_fd);
    char buffer[unp::MaxLine];
    
    while (true) {
        // 不要处理异常
        auto n = ::read(connected_fd, buffer, unp::MaxLine);
        if (n == unp::InvalidResult && errno == EINTR)
            continue;
        else if (n == unp::InvalidResult)
            unp::system_error("str_echo: read error");
        else if (n == 0)
            break;
        else {
            fmt::println("received: {}", buffer);
            unp::write(connected_fd, buffer, n);
        }
    }

    fmt::println("close connection {}", connected_fd);
}

void sig_child(int signo)
{   
    int state{};
    pid_t pid;
    while ((pid = ::waitpid(-1, &state, WNOHANG)) > 0) 
        fmt::println("child {} terminated.", pid);
}