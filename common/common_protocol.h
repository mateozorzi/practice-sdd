#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H


#include <atomic>
#include <string>
#include <vector>

#include <arpa/inet.h>

#include "isocket.h"
#include "socket.h"


class CommonProtocol {


protected:
    ISocket& socket;
    std::vector<uint8_t> protocol_buffer;
    // Socket::close keeps the fd number: closing twice could close a socket opened in between
    std::atomic<bool> stopped{false};

    void add_uint8_to_buffer(uint8_t value);
    void add_uint16_to_buffer(uint16_t value);
    void add_uint32_to_buffer(uint32_t value);
    void add_string_to_buffer(const std::string& str);
    void send_buffer();

    uint8_t recv_uint8();
    uint16_t recv_uint16();
    uint32_t recv_uint32();
    std::string recv_string();

    // Socket::sendall and Socket::recvall return 0 when the peer closed the connection
    void recv_exactly(void* data, unsigned int size);


public:
    CommonProtocol(const CommonProtocol& other) = delete;
    CommonProtocol& operator=(const CommonProtocol& other) = delete;

    explicit CommonProtocol(ISocket& socket);

    void stop();
};


#endif
