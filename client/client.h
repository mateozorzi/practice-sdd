#ifndef CLIENT_H
#define CLIENT_H
#include "client_protocol.h"

class Client {
public:
    explicit Client(const std::string& hostname, const std::string& servname);

    static void run();

private:
    ClientProtocol protocol;

};


#endif //CLIENT_H
