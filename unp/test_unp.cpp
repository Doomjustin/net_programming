#include <unp.h>

#include <cerrno>
#include <cstdlib>


int main(int argc, char* argv[])
{
    unp::message("{}", "test error");

    errno = EINTR;
    unp::error("{}", "test error");

    return EXIT_SUCCESS;
}