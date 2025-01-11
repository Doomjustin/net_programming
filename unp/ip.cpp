#include "unp.h"

#include <netinet/in.h>


namespace unp {

Endpoint IPv4::cast(const sockaddr_storage& address)
{
    const auto* ptr = reinterpret_cast<const sockaddr_in*>(&address);
    auto port = ::ntohs(ptr->sin_port);
    std::string addr(INET_ADDRSTRLEN, 0);
    unp::inet_ntop(AF_INET, &ptr->sin_addr, addr.data(), addr.size());

    if (ptr->sin_addr.s_addr == INADDR_LOOPBACK)
        addr = "localhost";
    else if (ptr->sin_addr.s_addr == INADDR_ANY)
        addr = "*";

    return Endpoint{ addr, port };
}

sockaddr_storage IPv4::cast(const Endpoint& endpoint)
{
    sockaddr_storage address{};
    auto addr = reinterpret_cast<sockaddr_in*>(&address);
    
    addr->sin_family = AF_INET;
    addr->sin_port = ::htons(endpoint.port());
    
    if (endpoint.address().empty() || endpoint.address() == "*") 
        addr->sin_addr.s_addr = ::htonl(INADDR_ANY);
    else if (to_lower(endpoint.address()) == "localhost")
        addr->sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
    else
        ::unp::inet_pton(AF_INET, endpoint.address().c_str(), &addr->sin_addr);

    return address;
}


Endpoint IPv6::cast(const sockaddr_storage& address)
{
    const auto* ptr = reinterpret_cast<const sockaddr_in6*>(&address);
    auto port = ::ntohs(ptr->sin6_port);
    std::string addr(INET6_ADDRSTRLEN, 0);
    unp::inet_ntop(AF_INET6, &ptr->sin6_addr, addr.data(), addr.size());
    
    /*
    if (ptr->sin6_addr == in6addr_loopback)
        addr = "localhost";
    else if (ptr->sin6_addr == in6addr_any)
        addr = "*";
    */

    return Endpoint{ addr, port };
}

sockaddr_storage IPv6::cast(const Endpoint& endpoint)
{
    sockaddr_storage address{};
    auto addr = reinterpret_cast<sockaddr_in6*>(&address);
    
    addr->sin6_family = AF_INET6;
    addr->sin6_port = ::htons(endpoint.port());
    
    if (endpoint.address().empty() || endpoint.address() == "*") 
        addr->sin6_addr = in6addr_any;
    else if (to_lower(endpoint.address()) == "localhost")
        addr->sin6_addr = in6addr_loopback;
    else
        ::unp::inet_pton(AF_INET6, endpoint.address().c_str(), &addr->sin6_addr);

    return address;
}

} // namespace unp