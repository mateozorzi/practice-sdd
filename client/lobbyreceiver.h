#ifndef LOBBYRECEIVER_H
#define LOBBYRECEIVER_H

#include "client_protocol.h"
#include "../common/thread.h"
#include <QObject>

class LobbyReceiver : public QObject, public Thread{

    Q_OBJECT

public:
    explicit LobbyReceiver(ClientProtocol& protocol, QObject* parent = nullptr);
    void run() override;

signals:
    void lobbyStarted();

private:
    ClientProtocol& protocol;
    bool start_lobby;
};


#endif // LOBBYRECEIVER_H