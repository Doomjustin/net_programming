#include <unp.h>

#include <iostream>
#include <string>
#include <cstdlib>

void echo(unp::Socket<unp::IPv4, unp::TCP> socket);


int main(int argc, char* argv[])
{
    unp::Socket<unp::IPv4, unp::TCP> socket{};

    unp::Endpoint target{ argv[1], unp::private_port::Echo };
    socket.connect(target);

    echo(std::move(socket));

    return EXIT_SUCCESS;
}


void echo(unp::Socket<unp::IPv4, unp::TCP> socket)
{
    std::cout << "start echo client\n";

    std::string line{};
    while (line != "quit") {
        std::cout << "> ";
        std::cin >> line;
        
        socket.write(line);
        auto received = socket.read(unp::MaxLine);
        if (received) {
            std::cout << *received << '\n';
        }
        else {
            std::cout << "Error: server closed\n";
            break;
        }
    }

    std::cout << "bye!\n";
}