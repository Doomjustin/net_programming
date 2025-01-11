#include "unp.h"

#include <cerrno>
#include <gsl/gsl>

#include <arpa/inet.h>
#include <cstdio>
#include <source_location>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>


namespace unp {

void check_result(int res, std::source_location cur)
{
    if (res == InvalidResult)
        system_error("failed to {}", cur.function_name());
}

int socket(int domain, int type, int protocol)
{
    auto res = ::socket(domain, type, protocol);
    check_result(res);

    return res;
}

void listen(int sockfd, int backlog)
{
    Expects(sockfd != InvalidFd);

    auto res = ::listen(sockfd, backlog);
    check_result(res);
}

void connect(int sockfd, gsl::not_null<const sockaddr*> addr, socklen_t addrlen)
{
    Expects(sockfd != InvalidFd);
    
    auto res = ::connect(sockfd, addr, addrlen);
    check_result(res);
}

void bind(int sockfd, gsl::not_null<const sockaddr*> addr, socklen_t addrlen)
{
    Expects(sockfd != InvalidFd);
    
    auto res = ::bind(sockfd, addr, addrlen);
    check_result(res);
}

int accept(int sockfd, sockaddr* addr, socklen_t* addrlen)
{
    Expects(sockfd != InvalidFd);

    while (true) {
        auto res = ::accept(sockfd, addr, addrlen);
        if (res == InvalidResult) {
            if (errno == EINTR || errno == ECONNABORTED) {
                error("failed to accept {} and retry.", sockfd);
                continue;
            }

            system_error("failed to accept {}.", sockfd);
        }
        return res;
    }
}

void inet_pton(int af, gsl::not_null<const char*> src, gsl::not_null<void*> dst)
{
    auto res = ::inet_pton(af, src, dst);
    check_result(res);
}

void inet_ntop(int af, gsl::not_null<const void*> src, gsl::not_null<char*> dst, socklen_t size)
{
    auto res = ::inet_ntop(af, src, dst, size);
    check_result(res == nullptr ? InvalidResult: 1);
}

void close(int fd)
{
    Expects(fd != InvalidFd);

    auto res = ::close(fd);
    check_result(res);
}

ssize_t read(int fd, gsl::not_null<void*> buf, size_t count)
{
    Expects(fd != InvalidFd);

    ssize_t res;

    while (true) {
        res = ::read(fd, buf, count);
        if (res == InvalidResult) {
            if (errno == EINTR) {
                error("failed to read {} and retry", fd);
                continue;
            }
    
            system_error("failed to read {}", fd);
        }

        return res;
    }
}

ssize_t write(int fd, gsl::not_null<const void*> buf, size_t count)
{
    Expects(fd != InvalidFd);

    while (true) {
        auto res = ::write(fd, buf, count);

        if (res == InvalidResult) {
            if (errno == EINTR) {
                error("failed to write {} and retry.", fd);
                continue;
            }

            system_error("faild to write {}.", fd);
        }

        return res;
    }
}

void getsockname(int sockfd, sockaddr* addr, socklen_t* addrlen)
{
    Expects(sockfd != InvalidFd);

    auto res = ::getsockname(sockfd, addr, addrlen);
    check_result(res);
}

void getpeername(int sockfd, sockaddr* addr, socklen_t* addrlen)
{
    Expects(sockfd != InvalidFd);

    auto res = ::getpeername(sockfd, addr, addrlen);
    check_result(res);
}

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout)
{
    auto res = ::select(nfds, readfds, writefds, exceptfds, timeout);
    check_result(res);

    return res;
}

void shutdown(int sockfd, int how)
{
    auto res = ::shutdown(sockfd, how);
    check_result(res);
}

} // namespace unp
