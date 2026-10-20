#include <iostream>
#include <streambuf>

#include <gtest/gtest.h>

namespace {

class NullBuffer: public std::streambuf {
protected:
    int overflow(int c) override { return traits_type::not_eof(c); }
};

}  // namespace

// The server threads log to std::cerr every time they stop ("[LOBBY] Stopping.", ...), which
// buries the test results. Only the C++ stream is silenced: gtest writes with stdio and crash
// messages (terminate, sanitizers) go straight to the file descriptor, so they still show.
int main(int argc, char** argv) {
    static NullBuffer discarded;
    std::cerr.rdbuf(&discarded);
    std::clog.rdbuf(&discarded);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
