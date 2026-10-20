#include "common_protocol.h"

#include <cerrno>
#include <cstring>
#include <vector>

#include "liberror.h"


CommonProtocol::CommonProtocol(ISocket& socket): socket(socket) {}


void CommonProtocol::add_uint8_to_buffer(const uint8_t value) {

    const size_t old_size = protocol_buffer.size();
    protocol_buffer.resize(old_size + sizeof(value));

    std::memcpy(protocol_buffer.data() + old_size, &value, sizeof(value));
}

void CommonProtocol::add_uint16_to_buffer(uint16_t value) {

    const uint16_t value_to_send = htons(value);
    const size_t old_size = protocol_buffer.size();
    protocol_buffer.resize(old_size + sizeof(value_to_send));

    std::memcpy(protocol_buffer.data() + old_size, &value_to_send, sizeof(value_to_send));
}

void CommonProtocol::add_uint32_to_buffer(uint32_t value) {

    const uint32_t value_to_send = htonl(value);
    const size_t old_size = protocol_buffer.size();
    protocol_buffer.resize(old_size + sizeof(value_to_send));

    std::memcpy(protocol_buffer.data() + old_size, &value_to_send, sizeof(value_to_send));
}

void CommonProtocol::add_string_to_buffer(const std::string& str) {

    const uint32_t str_size = str.size();
    add_uint32_to_buffer(str_size);
    const size_t old_size = protocol_buffer.size();
    protocol_buffer.resize(old_size + str_size);

    std::memcpy(protocol_buffer.data() + old_size, str.data(), str_size);
}

void CommonProtocol::send_buffer() {

    if (!protocol_buffer.empty()) {

        const int sent = socket.sendall(protocol_buffer.data(), protocol_buffer.size());
        protocol_buffer.clear();
        if (sent == 0)
            throw LibError(EPIPE, "the connection was closed while sending: ");
    }
}


uint8_t CommonProtocol::recv_uint8() {

    uint8_t value_to_recv = 0;
    recv_exactly(&value_to_recv, sizeof(value_to_recv));

    return value_to_recv;
}

uint16_t CommonProtocol::recv_uint16() {

    uint16_t value_to_recv = 0;
    recv_exactly(&value_to_recv, sizeof(value_to_recv));

    return ntohs(value_to_recv);
}

uint32_t CommonProtocol::recv_uint32() {

    uint32_t value_to_recv = 0;
    recv_exactly(&value_to_recv, sizeof(value_to_recv));

    return ntohl(value_to_recv);
}

std::string CommonProtocol::recv_string() {

    const uint32_t string_len = recv_uint32();
    std::vector<char> buf(string_len);
    recv_exactly(buf.data(), string_len);

    return std::string(buf.begin(), buf.end());
}

void CommonProtocol::recv_exactly(void* data, const unsigned int size) {

    if (size > 0 && socket.recvall(data, size) == 0)
        throw LibError(EPIPE, "the connection was closed while receiving: ");
}


void CommonProtocol::stop() {

    if (stopped.exchange(true))
        return;

    socket.shutdown(SHUT_RDWR);
    socket.close();
}
