#ifndef TEST_CLIENT_H
#define TEST_CLIENT_H

#include <iostream>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../client/client_protocol.h"
#include "../common/car_dto.h"
#include "../common/client_event_dto.h"
#include "../common/common_protocol.h"
#include "../common/lobby_dto.h"

#include "mock_socket.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class ClientProtocolTest: public ::testing::Test {
protected:
    void SetUp() override;

    void TearDown() override;

    ~ClientProtocolTest() override = default;

    // Variables que los tests usarán
    std::unique_ptr<MockSocket> mock_socket;
    std::unique_ptr<ClientProtocol> protocol;
};

#endif
