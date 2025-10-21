#ifndef PROTOCOL_H
#define PROTOCOL_H


#include <string>
#include <vector>

#include <arpa/inet.h>

#include "socket.h"


class CommonProtocol {

protected:
    Socket& socket;
    std::vector<uint8_t> protocol_buffer;

    void add_uint8_to_buffer(uint8_t value);
    void add_uint16_to_buffer(uint16_t value);
    void add_uint32_to_buffer(uint32_t value);
    void add_string_to_buffer(const std::string& str);
    void send_buffer();

    uint8_t recv_uint8();
    uint16_t recv_uint16();
    uint32_t recv_uint32();
    std::string recv_string();


public:
    CommonProtocol(const CommonProtocol& other) = delete;
    CommonProtocol& operator=(const CommonProtocol& other) = delete;

    explicit CommonProtocol(Socket& socket);
};


#endif
