#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H


#include <memory>
#include <string>

#include "../common/constants.h"
#include "../common/socket.h"

#include "receiver.h"
#include "sender.h"
#include "server_config.h"


class ClientHandler final: public Thread {


    Socket peer_socket;
    ServerProtocol server_protocol;
    Lobby& lobby;
    const ServerConfig& config;
    const uint32_t client_id;
    std::string username = "Player";
    uint32_t game_id = 0;
    bool is_in_lobby = true;

    void launch_lobby();

    void update_player_username(const std::string& actual_username);
    void list_active_games();

    void exit_lobby_and_play(Queue<GameCommand>& command_queue, Queue<WorldSnapshot>& snapshot_queue);
    void create_new_game(uint32_t client_id, uint8_t max_players);
    void join_existing_game(uint32_t client_id, uint32_t actual_game_id);


public:
    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

    explicit ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id, const ServerConfig& config);


    void run() override;

    void stop() override;

    ~ClientHandler();
};


#endif
