#ifndef LOBBYRECEIVER_H
#define LOBBYRECEIVER_H

#include <QObject>

#include "../common/thread.h"

#include "client_protocol.h"

class LobbyReceiver: public QObject, public Thread {

    Q_OBJECT

public:
    explicit LobbyReceiver(ClientProtocol& protocol, QObject* parent = nullptr);
    void run() override;

signals:
    void lobbyStarted();
    void lobbyDeleted();

private:
    ClientProtocol& protocol;
    bool start_lobby;
};


#endif  // LOBBYRECEIVER_H
