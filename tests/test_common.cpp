#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include <gtest/gtest.h>
#include <sys/socket.h>

#include "../common/liberror.h"
#include "../common/queue.h"
#include "../common/resolver.h"
#include "../common/resolvererror.h"
#include "../common/socket.h"
#include "../common/thread.h"
#include "../common/upgrade_dto.h"

namespace {

std::atomic<int> next_port{25500};

// Listening socket on a fresh port plus a client connected to it
struct SocketPair {
    std::string port = std::to_string(next_port++);
    Socket acceptor{port.c_str()};
    Socket client{"localhost", port.c_str()};
    Socket server = acceptor.accept();
};

class FlagThread: public Thread {
public:
    std::atomic<bool> ran{false};
    bool throw_in_run = false;

    void run() override {
        ran = true;
        if (throw_in_run)
            throw std::runtime_error("boom");
        while (should_keep_running()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
};

class UnknownExceptionThread: public Thread {
public:
    void run() override { throw 42; }
};

}  // namespace


// Queue

TEST(QueueTest, KeepsFifoOrder) {
    Queue<int> queue(10);

    queue.push(1);
    queue.try_push(2);
    queue.push(3);

    int value = 0;
    EXPECT_EQ(queue.pop(), 1);
    ASSERT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, 2);
    EXPECT_EQ(queue.pop(), 3);
    EXPECT_FALSE(queue.try_pop(value));
}

TEST(QueueTest, TryPushFailsWhenFull) {
    Queue<int> queue(2);

    EXPECT_TRUE(queue.try_push(1));
    EXPECT_TRUE(queue.try_push(2));
    EXPECT_FALSE(queue.try_push(3));

    int value = 0;
    ASSERT_TRUE(queue.try_pop(value));
    EXPECT_TRUE(queue.try_push(3));
}

TEST(QueueTest, DefaultQueueIsUnbounded) {
    Queue<int> queue;

    for (int i = 0; i < 1000; ++i) ASSERT_TRUE(queue.try_push(i));
}

TEST(QueueTest, ClosedQueueRejectsPushesAndDrainsBeforeFailing) {
    Queue<int> queue(10);
    queue.push(7);

    queue.close();

    EXPECT_THROW(queue.push(1), ClosedQueue);
    EXPECT_THROW(queue.try_push(1), ClosedQueue);
    EXPECT_EQ(queue.pop(), 7);
    int value = 0;
    EXPECT_THROW(queue.try_pop(value), ClosedQueue);
    EXPECT_THROW(queue.pop(), ClosedQueue);
}

TEST(QueueTest, ClosingTwiceThrows) {
    Queue<int> queue(1);
    queue.close();

    EXPECT_THROW(queue.close(), std::runtime_error);
}

TEST(QueueTest, PopWaitsForAPush) {
    Queue<int> queue(1);

    std::thread producer([&queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.push(5);
    });

    EXPECT_EQ(queue.pop(), 5);
    producer.join();
}

TEST(QueueTest, PushWaitsUntilThereIsRoom) {
    Queue<int> queue(1);
    queue.push(1);
    std::atomic<bool> pushed{false};

    std::thread producer([&]() {
        queue.push(2);
        pushed = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_FALSE(pushed);

    EXPECT_EQ(queue.pop(), 1);
    producer.join();
    EXPECT_TRUE(pushed);
    EXPECT_EQ(queue.pop(), 2);
}

TEST(QueueTest, CloseWakesUpABlockedPop) {
    Queue<int> queue(1);

    std::thread closer([&queue]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.close();
    });

    EXPECT_THROW(queue.pop(), ClosedQueue);
    closer.join();
}

TEST(QueueTest, PointerSpecializationBehavesTheSame) {
    Queue<void*> queue(1);
    int a = 1;
    int b = 2;
    void* value = nullptr;

    EXPECT_FALSE(queue.try_pop(value));
    EXPECT_TRUE(queue.try_push(&a));
    EXPECT_FALSE(queue.try_push(&b));
    EXPECT_EQ(queue.pop(), &a);

    queue.push(&b);
    ASSERT_TRUE(queue.try_pop(value));
    EXPECT_EQ(value, &b);

    std::thread producer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        queue.push(&a);
    });
    EXPECT_EQ(queue.pop(), &a);
    producer.join();

    queue.push(&a);
    std::thread consumer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        queue.pop();
    });
    queue.push(&b);  // waits until the consumer makes room
    consumer.join();

    queue.close();
    EXPECT_THROW(queue.push(&a), ClosedQueue);
    EXPECT_THROW(queue.try_push(&a), ClosedQueue);
    EXPECT_EQ(queue.pop(), &b);
    EXPECT_THROW(queue.try_pop(value), ClosedQueue);
    EXPECT_THROW(queue.pop(), ClosedQueue);
    EXPECT_THROW(queue.close(), std::runtime_error);
}


// Socket and Resolver

TEST(SocketTest, SendsAndReceivesExactSizes) {
    SocketPair pair;
    const char message[] = "need for speed";

    EXPECT_EQ(pair.client.sendall(message, sizeof(message)), static_cast<int>(sizeof(message)));

    char buffer[sizeof(message)] = {};
    EXPECT_EQ(pair.server.recvall(buffer, sizeof(buffer)), static_cast<int>(sizeof(buffer)));
    EXPECT_STREQ(buffer, message);
}

TEST(SocketTest, RecvsomeReturnsWhatIsAvailable) {
    SocketPair pair;
    pair.server.sendall("abc", 3);

    char buffer[16] = {};
    int received = pair.client.recvsome(buffer, sizeof(buffer));

    EXPECT_GT(received, 0);
    EXPECT_LE(received, 3);
}

TEST(SocketTest, ClosedPeerIsDetected) {
    SocketPair pair;
    pair.client.shutdown(SHUT_RDWR);
    pair.client.close();

    char byte = 0;
    EXPECT_EQ(pair.server.recvall(&byte, 1), 0);
    EXPECT_TRUE(pair.server.is_stream_recv_closed());
}

TEST(SocketTest, PeerClosingMidMessageThrows) {
    SocketPair pair;
    pair.client.sendall("ab", 2);
    pair.client.shutdown(SHUT_RDWR);

    char buffer[4] = {};
    EXPECT_THROW(pair.server.recvall(buffer, sizeof(buffer)), LibError);
}

TEST(SocketTest, SendingToAClosedPeerClosesTheSendStream) {
    SocketPair pair;
    pair.client.shutdown(SHUT_RDWR);
    pair.client.close();

    // The first sends may be buffered by the OS: keep sending until the broken pipe shows up
    char buffer[4096] = {};
    bool failed = false;
    for (int i = 0; i < 1000 && !failed; ++i) {
        try {
            failed = pair.server.sendall(buffer, sizeof(buffer)) == 0;
        } catch (const LibError&) {
            failed = true;
        }
    }

    EXPECT_TRUE(failed);
}

TEST(SocketTest, ShutdownMarksTheStreams) {
    SocketPair pair;

    EXPECT_FALSE(pair.client.is_stream_send_closed());
    EXPECT_FALSE(pair.client.is_stream_recv_closed());

    pair.client.shutdown(SHUT_WR);
    EXPECT_TRUE(pair.client.is_stream_send_closed());
    EXPECT_FALSE(pair.client.is_stream_recv_closed());

    pair.server.shutdown(SHUT_RD);
    EXPECT_TRUE(pair.server.is_stream_recv_closed());

    pair.server.shutdown(SHUT_RDWR);
    EXPECT_TRUE(pair.server.is_stream_send_closed());
}

TEST(SocketTest, InvalidShutdownModeThrows) {
    SocketPair pair;

    EXPECT_ANY_THROW(pair.client.shutdown(42));
}

TEST(SocketTest, MovedSocketKeepsTheConnection) {
    SocketPair pair;

    Socket moved(std::move(pair.client));
    EXPECT_THROW(pair.client.sendall("x", 1), std::runtime_error);

    // Assigning over an open socket closes it first
    const std::string other_port = std::to_string(next_port++);
    Socket assigned(other_port.c_str());
    assigned = std::move(moved);
    Socket& same = assigned;
    assigned = std::move(same);  // self assignment is a no-op

    char byte = 0;
    assigned.sendall("z", 1);
    pair.server.recvall(&byte, 1);
    EXPECT_EQ(byte, 'z');
}

TEST(SocketTest, ConnectingToAClosedPortFails) {
    const std::string port = std::to_string(next_port++);

    EXPECT_THROW(Socket("localhost", port.c_str()), LibError);
}

TEST(SocketTest, ListeningTwiceOnThePortFails) {
    const std::string port = std::to_string(next_port++);
    Socket first(port.c_str());

    EXPECT_THROW(Socket second(port.c_str()), LibError);
}

TEST(SocketTest, AcceptOnAClosedSocketFails) {
    const std::string port = std::to_string(next_port++);
    Socket acceptor(port.c_str());
    acceptor.shutdown(SHUT_RDWR);
    acceptor.close();

    EXPECT_THROW(acceptor.accept(), LibError);
}

TEST(SocketTest, UnknownServiceFails) {
    EXPECT_THROW(Socket("localhost", "not-a-real-service"), ResolverError);
}

TEST(ResolverTest, ResolvesLocalhost) {
    Resolver resolver("localhost", "80", false);

    ASSERT_TRUE(resolver.has_next());
    EXPECT_NE(resolver.next(), nullptr);
}

TEST(ResolverTest, MovedResolverCannotBeUsed) {
    Resolver resolver("localhost", "80", false);

    Resolver moved(std::move(resolver));
    EXPECT_THROW(resolver.has_next(), std::runtime_error);
    EXPECT_TRUE(moved.has_next());

    Resolver assigned(nullptr, "80", true);
    assigned = std::move(moved);
    Resolver& same = assigned;
    assigned = std::move(same);  // self assignment is a no-op
    EXPECT_TRUE(assigned.has_next());
    EXPECT_THROW(moved.next(), std::runtime_error);
}

TEST(ResolverTest, UnknownServiceThrowsResolverError) {
    try {
        Resolver resolver("localhost", "not-a-real-service", false);
        FAIL() << "expected ResolverError";
    } catch (const ResolverError& error) {
        EXPECT_GT(std::strlen(error.what()), 0u);
    }
}


// Errors, Thread and DTOs

TEST(LibErrorTest, FormatsTheMessageAndTheErrno) {
    LibError error(ECONNREFUSED, "connect to %s:%d ", "localhost", 8080);

    const std::string message = error.what();
    EXPECT_EQ(message.rfind("connect to localhost:8080 ", 0), 0u);
    EXPECT_NE(message.find(std::strerror(ECONNREFUSED)), std::string::npos);
}

TEST(ThreadTest, RunsUntilStopped) {
    FlagThread thread;
    EXPECT_FALSE(thread.is_alive());

    thread.start();
    while (!thread.ran) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_TRUE(thread.is_alive());

    thread.stop();
    thread.join();
    EXPECT_FALSE(thread.is_alive());
}

TEST(ThreadTest, ExceptionsInRunEndTheThread) {
    FlagThread thread;
    thread.throw_in_run = true;

    // The tests' main silences std::cerr, so it is redirected here to check the messages
    std::ostringstream errors;
    std::streambuf* silenced = std::cerr.rdbuf(errors.rdbuf());
    thread.start();
    thread.join();

    EXPECT_TRUE(thread.ran);
    EXPECT_FALSE(thread.is_alive());

    UnknownExceptionThread unknown;
    unknown.start();
    unknown.join();
    EXPECT_FALSE(unknown.is_alive());

    std::cerr.rdbuf(silenced);
    EXPECT_NE(errors.str().find("Unexpected exception: boom"), std::string::npos);
    EXPECT_NE(errors.str().find("Unexpected exception: <unknown>"), std::string::npos);
}

TEST(UpgradeDTOTest, StoresTheIncreases) {
    UpgradeDTO none;
    EXPECT_EQ(none.handling_increase, 0);
    EXPECT_EQ(none.health_increase, 0);

    UpgradeDTO upgrade(1, 2, 3, 400);
    EXPECT_EQ(upgrade.handling_increase, 1);
    EXPECT_EQ(upgrade.speed_increase, 2);
    EXPECT_EQ(upgrade.acceleration_increase, 3);
    EXPECT_EQ(upgrade.health_increase, 400);
}
