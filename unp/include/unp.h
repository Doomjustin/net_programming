#ifndef FORWARD_UNP_H
#define FORWARD_UNP_H

#include <condition_variable>
#include <cstddef>
#include <gsl/gsl>
#include <fmt/core.h>

#include <gsl/pointers>
#include <mutex>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <utility>
#include <string>
#include <span>
#include <source_location>
#include <coroutine>


namespace unp {

[[nodiscard("to_upper output")]]
std::string to_upper(const std::string_view in);

[[nodiscard("to_lower output")]]
std::string to_lower(const std::string_view in);

template<typename... Args>
void output_error_message(fmt::format_string<Args...> fmt, Args&&... args)
{
    static constexpr auto* MessageFmt = "Error({}): {}: {}";
    auto current_error = errno;

    fmt::println(MessageFmt, 
                  current_error, 
                  std::strerror(current_error), 
                  fmt::format(std::move(fmt), std::forward<Args>(args)...)
                );
}

template<typename... Args>
void error(fmt::format_string<Args...> fmt, Args&&... args)
{
    output_error_message(std::move(fmt), std::forward<Args>(args)...);
}

template<typename... Args>
void system_error(fmt::format_string<Args...> fmt, Args&&... args)
{
    output_error_message(std::move(fmt), std::forward<Args>(args)...);
    std::exit(1);
}

template<typename... Args>
void dump_error(fmt::format_string<Args...> fmt, Args&&... args)
{
    output_error_message(std::move(fmt), std::forward<Args>(args)...);
    std::abort();
    std::exit(1);
}

template<typename... Args>
void message(fmt::format_string<Args...> fmt, Args&&... args)
{
    fmt::println(std::move(fmt), std::forward<Args>(args)...);
}

void check_result(int res, std::source_location cur = std::source_location::current());


enum private_port: std::uint16_t {
    Daytime = 49152,
    Echo
};

static constexpr int MaxLine = 4096;

static constexpr int InvalidFd = -1;
static constexpr int InvalidResult = -1;

int socket(int domain, int type, int protocol);

void listen(int sockfd, int backlog);

void connect(int sockfd, gsl::not_null<const sockaddr*> addr, socklen_t addrlen);

void bind(int sockfd, gsl::not_null<const sockaddr*> addr, socklen_t addrlen);

int accept(int sockfd, sockaddr* addr, socklen_t* addrlen);

void inet_pton(int af, gsl::not_null<const char*> src, gsl::not_null<void*> dst);

void inet_ntop(int af, gsl::not_null<const void*> src, gsl::not_null<char*> dst, socklen_t size);

void close(int fd);

ssize_t read(int fd, gsl::not_null<void*> buf, size_t count);

ssize_t write(int fd, gsl::not_null<const void*> buf, size_t count);

std::string readline(int fd, char endline = '\n');

int fputc(int c, gsl::not_null<FILE*> stream);

void getsockname(int sockfd, sockaddr* addr, socklen_t* addrlen);

void getpeername(int sockfd, sockaddr* addr, socklen_t* addrlen);

pid_t fork();

void sig_child(int signo);

int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout);

void shutdown(int sockfd, int how);


class Endpoint {
public:
    using PortType = std::uint16_t;

    explicit Endpoint(PortType port)
      : port_{ port }
    {}

    explicit Endpoint(std::string address, PortType port = 0)
      : address_{ std::move(address) }, port_{ port }
    {}

    std::string address() const noexcept { return address_; }

    constexpr PortType port() const noexcept { return port_; }

private:
    std::string address_{};
    PortType port_ = 0;
};


struct IPv4 {
    static Endpoint cast(const sockaddr_storage& address);

    static sockaddr_storage cast(const Endpoint& endpoint);

    static constexpr auto domain() { return AF_INET; }
};

struct IPv6 {
    static Endpoint cast(const sockaddr_storage& address);

    static sockaddr_storage cast(const Endpoint& endpoint);

    static constexpr auto domain() { return AF_INET6; }
};


struct TCP {
    static constexpr auto family() { return SOCK_STREAM; }
};

struct UDP {
    static constexpr auto family() { return SOCK_DGRAM; }
};


template<typename IP, typename TP>
class Socket;

template<typename IP>
class Socket<IP, TCP> {
public:
    Socket()
    {
        sock_ = unp::socket(IP::domain(), TCP::family(), 0);
        Ensures(valid());
    }

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept
      : sock_{ std::exchange(other.sock_, InvalidFd) }
    {
        Ensures(valid());
    }

    Socket& operator=(Socket&& other) noexcept
    {
        if (this == &other) return *this;

        // 忽略close引发的错误
        if (valid()) ::close(sock_);

        sock_ = std::exchange(other.sock_, InvalidFd);
        return *this;
    }

    ~Socket()
    {
        if (valid()) {
            ::close(sock_);
            sock_ = InvalidFd;
        }
    }

    constexpr bool valid() const noexcept { return sock_ != InvalidFd; }

    void close()
    {
        Expects(valid());

        unp::close(sock_);
        sock_ = InvalidFd;
    }

    void connect(const Endpoint& peer)
    {
        Expects(valid());

        peer_ = peer;
        auto address = IP::cast(peer);
        unp::connect(sock_, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    }

    void listen(int backlog = 5) const
    {
        Expects(valid());
        unp::listen(sock_, backlog);
    }

    void bind(const Endpoint& self)
    {
        Expects(valid());

        self_ = self;
        auto address = IP::cast(self);
        unp::bind(sock_, reinterpret_cast<sockaddr*>(&address), sizeof(address));
    }

    Socket accept()
    {
        Expects(valid());

        sockaddr_storage address{};
        socklen_t length = sizeof(address);
        auto accept_sock = unp::accept(sock_, reinterpret_cast<sockaddr*>(&address), &length);
        
        Socket peer_socket{ accept_sock };
        peer_socket.self_ = self_;
        peer_socket.peer_ = IP::cast(address);

        return peer_socket;
    }

    ssize_t write(gsl::not_null<const void*> buf, size_t count)
    {
        return unp::write(sock_, buf, count);
    } 

    ssize_t write(std::span<char> buffer)
    {
        Expects(valid());

        auto write_bytes = unp::write(sock_, buffer.data(), buffer.size());
        return write_bytes;
    }

    std::optional<std::string> read(ssize_t max_count)
    {
        Expects(valid());

        std::string buffer(max_count, '\0');
        auto read_bytes = unp::read(sock_, buffer.data(), buffer.size());
        
        Ensures(read_bytes != InvalidResult);
        if (read_bytes == 0) return {};

        return buffer.substr(0, read_bytes);
    }

    ssize_t read(gsl::not_null<void*> buf, size_t count)
    {
        return unp::read(sock_, buf, count);
    }

    void shutdown(int how)
    {
        unp::shutdown(sock_, how);
    }

    Endpoint self()
    {
        if (self_) return *self_;

        Expects(valid());
        sockaddr_storage address{};
        socklen_t length{};
        unp::getsockname(sock_, reinterpret_cast<sockaddr*>(&address), &length);
        self_ = IP::cast(address);

        return *self_;
    }

    Endpoint peer()
    {
        if (peer_) return *peer_;

        Expects(valid());
        sockaddr_storage address{};
        socklen_t length{};
        unp::getpeername(sock_, reinterpret_cast<sockaddr*>(&address), &length);
        peer_ = IP::cast(address);

        return *peer_;
    }

    operator bool() const noexcept
    {
        return valid();
    }

private:
    int sock_ = InvalidFd;
    std::optional<Endpoint> self_;
    std::optional<Endpoint> peer_;

    explicit Socket(int sock)
      : sock_{ sock }
    {
        Ensures(valid());
    }
};

} // namespace unp

#endif // FORWARD_UNP_H