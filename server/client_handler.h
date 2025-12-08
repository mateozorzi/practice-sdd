#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H


#include <memory>
#include <string>

#include "../common/constants.h"
#include "../common/race_catalogue_dto.h"
#include "../common/socket.h"

#include "receiver.h"
#include "sender.h"
#include "server_config.h"


class ClientHandler final: public Thread {


    Socket peer_socket;
    ServerProtocol server_protocol;
    Lobby& lobby;
    const ServerConfig& server_config;
    const uint32_t player_id;
    std::string username = std::string(DEFAULT_USERNAME);
    bool is_in_lobby = true;
    RaceCatalogueDTO selected_races;

    void launch_lobby();

    void update_player_username(const std::string& actual_username);
    void send_car_catalogue();
    void list_active_games();
    void list_loaded_races();
    void select_races(const LobbyDTO& lobby_dto);

    void exit_lobby_and_play(Queue<GameCommand>& command_queue,
                             Queue<GameUpdateDTO>& snapshot_queue);
    void create_new_game(uint8_t max_players, const std::string& game_name);
    void join_existing_game(uint32_t game_id);

    void exit_game();


public:
    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

    explicit ClientHandler(Socket&& peer_socket, Lobby& lobby, uint32_t client_id,
                           const ServerConfig& config);


    void run() override;

    void stop() override;

    ~ClientHandler();
};


#endif
