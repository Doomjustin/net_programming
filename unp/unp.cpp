#include "unp.h"

#include <ranges>
#include <unistd.h>
#include <sys/wait.h>


namespace unp {

[[nodiscard("to_upper output")]]
std::string to_upper(const std::string_view in)
{
    auto view = in | std::views::transform([] (const char c) { return std::toupper(c); });
    return { view.begin(), view.end() };
}

[[nodiscard("to_lower output")]]
std::string to_lower(const std::string_view in)
{
    auto view = in | std::views::transform([] (const char c) { return std::tolower(c); });
    return { view.begin(), view.end() };
}

std::string readline(int fd, char endline)
{
    std::string line{};
    ssize_t n = 0;
    char c{};
    while ((n = unp::read(fd, &c, 1)) == 1) {
        if (c == endline) break;
        else line.push_back(c);
    }

    return line;
}

int fputc(int c, gsl::not_null<FILE*> stream)
{
    auto res = ::fputc(c, stream);
    if (res == EOF)
        error("failed to fputc {}", c);

    return res;
}

pid_t fork()
{
    auto res = ::fork();
    check_result(res);

    return res;
}

void sig_child(int signo)
{   
    int state{};
    pid_t pid;
    while ((pid = ::waitpid(-1, &state, WNOHANG)) > 0);
}

} // namespace unp