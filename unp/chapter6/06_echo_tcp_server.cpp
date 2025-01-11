#include <algorithm>
#include <array>
#include <sys/select.h>
#include <unp.h>
#include <fmt/core.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>


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

    std::array<int, FD_SETSIZE> clients{};
    std::ranges::fill(clients, -1);

    fd_set all_sets{};
    FD_ZERO(&all_sets);
    FD_SET(socket, &all_sets);

    auto max_fds = socket;
    fd_set read_sets{};
    int max_index = -1;
    int sock_fd = -1;
    int n;
    char buffer[unp::MaxLine];

    sockaddr_in client_address{};
    socklen_t client_size = sizeof(client_address);
    while (true) {
        read_sets = all_sets;
        auto ready_fds = unp::select(max_fds + 1, &read_sets, nullptr, nullptr, nullptr);

        if (FD_ISSET(socket, &read_sets)) {
            auto connected_fd = unp::accept(socket, reinterpret_cast<sockaddr*>(&client_address), &client_size);
            
            int current_index = 0;
            for (auto& client: clients) {
                ++current_index;
                if (client == -1) {
                    client = connected_fd;
                    break;
                }
            }

            if (current_index == FD_SETSIZE) {
                unp::error("too many clients");
                std::exit(EXIT_FAILURE);
            }

            FD_SET(connected_fd, &all_sets);
            max_fds = std::max(max_fds, connected_fd);
            max_index = std::max(max_index, current_index);

            if (--ready_fds <= 0) continue;
        }   

        for (int i = 0; i <= max_index; ++i) {
            if ((sock_fd = clients[i]) == -1) continue;

            if (FD_ISSET(sock_fd, &read_sets)) {
                if ((n = unp::read(sock_fd, buffer, unp::MaxLine)) == 0) {
                    unp::close(sock_fd);
                    FD_CLR(sock_fd, &all_sets);
                    clients[i] = -1;
                }
                else {
                    unp::write(sock_fd, buffer, n);
                }

                if (--ready_fds <= 0)
                    break;
            }
        }
    }

    return EXIT_SUCCESS;
}


void sig_child(int signo)
{   
    int state{};
    pid_t pid;
    while ((pid = ::waitpid(-1, &state, WNOHANG)) > 0) 
        fmt::println("child {} terminated.", pid);
}