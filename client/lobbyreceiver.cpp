#include "lobbyreceiver.h"

LobbyReceiver::LobbyReceiver(ClientProtocol& protocol, QObject* parent)
    : QObject(parent), protocol(protocol), start_lobby(false) {}

void LobbyReceiver::run() {
    while (should_keep_running()) {
        try {
            // Here you would implement the logic to receive lobby updates

            //client receive start_lobby
            //start_lobby = protocol.receive_lobby_start();

            LobbyDTO lobby_update = protocol.receive_lobby_action();

            if (lobby_update.action == LobbyAction::START_LOBBY) {
                emit lobbyStarted();
                break;
            } else if (lobby_update.action == LobbyAction::LEAVE_LOBBY ) {
                emit lobbyDeleted();
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }catch(const std::exception& e){
            break;
        }
    }
}