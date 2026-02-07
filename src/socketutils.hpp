#pragma once

#include "logger.hpp"
#include <cstddef>
#include <stdexcept>
#include <sys/socket.h>

namespace MinimalCoupler
{
namespace utils
{

inline void sendAll(int socket, const void *data, size_t length)
{
    const char *ptr = static_cast<const char *>(data);
    size_t sent = 0;
    while (sent < length)
    {
        ssize_t n = send(socket, ptr + sent, length - sent, 0);
        if (n <= 0)
            throw std::runtime_error("sendAll: connection error");
        sent += static_cast<size_t>(n);
        MINIMALCOUPLER_INFO("Sent ", sent, " data");
    }
}

inline void recvAll(int socket, void *data, size_t length)
{
    char *ptr = static_cast<char *>(data);
    size_t received = 0;
    while (received < length)
    {
        ssize_t n = recv(socket, ptr + received, length - received, 0);
        if (n <= 0)
            throw std::runtime_error("recvAll: connection error");
        received += static_cast<size_t>(n);
        MINIMALCOUPLER_INFO("Received ", received, " data");
    }
}

} // namespace utils
} // namespace MinimalCoupler
