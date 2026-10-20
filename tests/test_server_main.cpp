#include <atomic>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include <ext/stdio_filebuf.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "../common/common_protocol.h"
#include "../common/lobby_dto.h"
#include "../common/socket.h"
#include "../server/server.h"

#include "server_test_utils.h"

// Integration tests of Server: it runs until 'q' is read from the standard input, which the
// tests feed through a pipe.

namespace {

std::atomic<int> next_port{27500};

// Replaces std::cin with the read end of a pipe while alive
class StdinPipe {
    int fds[2];
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> buffer;
    std::streambuf* original;

public:
    StdinPipe() {
        if (pipe(fds) != 0)
            throw std::runtime_error("pipe failed");
        buffer = std::make_unique<__gnu_cxx::stdio_filebuf<char>>(fds[0], std::ios::in);
        original = std::cin.rdbuf(buffer.get());
    }

    StdinPipe(const StdinPipe&) = delete;
    StdinPipe& operator=(const StdinPipe&) = delete;

    void type(const std::string& text) {
        if (write(fds[1], text.data(), text.size()) != static_cast<ssize_t>(text.size()))
            throw std::runtime_error("write failed");
    }

    ~StdinPipe() {
        std::cin.rdbuf(original);
        std::cin.clear();
        close(fds[1]);
        buffer.reset();  // closes fds[0]
    }
};

// Constructed before CommonProtocol, which keeps a reference to the socket
struct ConnectedSocket {
    Socket connection;
};

// Lobby client speaking the wire format by hand (the client headers cannot be included next
// to the server ones)
class RawClient: private ConnectedSocket, public CommonProtocol {

    void send_lobby_message(LobbyAction action, const std::string& text) {
        add_uint8_to_buffer(static_cast<uint8_t>(action));
        add_uint32_to_buffer(0);
        add_string_to_buffer(text);
        add_uint8_to_buffer(0);
        add_uint8_to_buffer(0);
        add_uint8_to_buffer(0);  // no races selected
        send_buffer();
    }

public:
    explicit RawClient(const std::string& port):
            ConnectedSocket{Socket("localhost", port.c_str())}, CommonProtocol(connection) {}

    uint32_t change_username(const std::string& username) {
        send_lobby_message(LobbyAction::CHANGE_USERNAME, username);
        return recv_uint32();
    }

    // Leaves and waits until the server closes the connection
    void leave() {
        send_lobby_message(LobbyAction::LEAVE_LOBBY, "");
        char byte;
        while (connection.recvsome(&byte, 1) > 0) {}
    }
};

class ServerTest: public ::testing::Test {
protected:
    ServerConfig config = make_test_server_config();
    std::string port = std::to_string(next_port++);
    StdinPipe stdin_pipe;
    std::ostringstream output;
    std::streambuf* original_cout = std::cout.rdbuf(output.rdbuf());

    void TearDown() override { std::cout.rdbuf(original_cout); }
};

}  // namespace


TEST_F(ServerTest, RunsUntilQIsTyped) {
    Server server(port, config);
    std::thread running([&server]() { server.run(); });

    stdin_pipe.type("hello\n");
    stdin_pipe.type("q\n");
    running.join();

    EXPECT_NE(output.str().find(SERVER_STARTED), std::string::npos);
    EXPECT_NE(output.str().find(SERVER_STOPPED), std::string::npos);
}

TEST_F(ServerTest, AcceptsClientsWhileRunning) {
    Server server(port, config);
    std::thread running([&server]() { server.run(); });

    {
        RawClient client(port);
        EXPECT_EQ(client.change_username("mateo"), FIRST_CLIENT_ID);
        client.leave();
    }

    stdin_pipe.type("q");
    running.join();
}

TEST_F(ServerTest, UsedPortFails) {
    Socket taken(port.c_str());

    EXPECT_ANY_THROW(Server(port, config));
}
