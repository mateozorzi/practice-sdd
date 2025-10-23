#include <chrono>
#include <thread>

#include "receiver.h"

#include "common/liberror.h"


Receiver::Receiver(const uint32_t client_id): client_id(client_id) {}


void Receiver::run() {

    try {

        while (should_keep_running()) {

            std::this_thread::sleep_for(std::chrono::seconds(3)); // SLEEP ONLY FOR NOW! We don't want a busy loop, we want to keep our CPU :)
            std::cout << RECEIVER_ACTIVATED << std::endl;
        }
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << RECEIVER_EXCEPTION << ": " << e.what();
    }
}


void Receiver::stop() {

    if (_keep_running) {

        _keep_running = false;
    }
}

Receiver::~Receiver() {

    try {

        Receiver::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << RECEIVER << "] " << RECEIVER_EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
