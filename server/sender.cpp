#include <thread>
#include <chrono>

#include "sender.h"

#include "common/liberror.h"


Sender::Sender(const uint32_t client_id): client_id(client_id) {}


void Sender::run() {

    try {

        while (should_keep_running()) {

            std::this_thread::sleep_for(std::chrono::seconds(3)); // SLEEP ONLY FOR NOW! We don't want a busy loop, we want to keep our CPU :)
            std::cout << SENDER_ACTIVATED << std::endl;
            /*try {

            } catch (const ClosedQueue&) {

                break;
            } NO QUEUE FOR NOW! */
        }
    } catch (const LibError&) {

    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << SENDER_EXCEPTION << ": " << e.what();
    }
}

void Sender::stop() {

    if (_keep_running) {

        _keep_running = false;
    }
}


Sender::~Sender() {

    try {

        Sender::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << SENDER << "] " << SENDER_EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
