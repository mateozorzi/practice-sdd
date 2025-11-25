#ifndef TEST_SERVER_H
#define TEST_SERVER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>

#include "../server/server_protocol.h"
#include "../common/common_protocol.h"
#include "mock_socket.h"
#include "../common/client_event_dto.h"
#include "../common/lobby_dto.h"
#include "../common/car_dto.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;

class ServerProtocolTest : public ::testing::Test {
protected:
    void SetUp() override;
    
    void TearDown() override;

    ~ServerProtocolTest() override = default;

    // Variables que los tests usarán
    std::unique_ptr<MockSocket> mock_socket;
    std::unique_ptr<ServerProtocol> protocol;
};

#endif