#ifndef MOCK_SOCKET_H
#define MOCK_SOCKET_H

#include <gmock/gmock.h>

#include "../common/isocket.h"

class MockSocket: public ISocket {
public:
    MOCK_METHOD(int, sendsome, (const void* data, unsigned int sz), (override));
    MOCK_METHOD(int, recvsome, (void* data, unsigned int sz), (override));
    MOCK_METHOD(int, sendall, (const void* data, unsigned int sz), (override));
    MOCK_METHOD(int, recvall, (void* data, unsigned int sz), (override));
    MOCK_METHOD(void, shutdown, (int how), (override));
    MOCK_METHOD(bool, is_stream_send_closed, (), (const, override));
    MOCK_METHOD(bool, is_stream_recv_closed, (), (const, override));
    MOCK_METHOD(int, close, (), (override));
};

#endif
