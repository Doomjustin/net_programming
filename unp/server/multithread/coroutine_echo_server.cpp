#include <fmt/core.h>
#include <unp.h>

#include <sys/wait.h>
#include <cstdlib>


unp::Channel echo_server()
{
    unp::Endpoint self{ unp::private_port::Echo };
    auto session = co_await unp::accept(self);
    auto message = co_await session.read(unp::MaxLine);
    co_await session.write(message.data(), message.size());
}


int main(int argc, char* argv[])
{
    signal(SIGCHLD, unp::sig_child);

    echo_server();

    return EXIT_SUCCESS;
}
