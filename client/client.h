#ifndef CLIENT_H
#define CLIENT_H
#include "client_protocol.h"
#include "receiver.h"
#include "sender.h"

class Client {
public:
    explicit Client(const std::string& hostname, const std::string& servname);
    void run();
    ~Client();

private:
    ClientProtocol protocol;
    Sender sender;
    Receiver receiver;
};


#endif //CLIENT_H
