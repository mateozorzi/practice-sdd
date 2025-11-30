#include "game_loop.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../common/checkpoint_dto.h"
#include "../common/client_event_dto.h"
#include "../common/constants.h"
#include "../common/game_update_dto.h"
#include "../common/leaderboard_dto.h"

#include "world_creation_exception.h"


GameLoop::GameLoop(GameMonitor& game_monitor, Queue<GameCommand>& command_queue,
                   const uint8_t max_players, const ServerConfig& config,
                   RaceCatalogueDTO& selected):
        config(config),
        max_players(max_players),
        game_iteration_duration(1000 / config.game.game_fps),
        upgrade_shop_seconds(config.game.wait_seconds_upgrade_shop),
        countdown_seconds(config.game.wait_seconds_countdown),
        race_timeout_seconds(config.game.timeout_seconds_race),
        leaderboard_seconds(config.game.wait_seconds_leaderboard),
        game_finish_seconds(config.game.wait_seconds_game_finished),
        game_monitor(game_monitor),
        command_queue(command_queue),
        city_loader(box2d_world),
        collision_listener(players),
        selected_races(selected) {

    box2d_world.SetContactListener(&collision_listener);
}


void GameLoop::fixed_sleep(const std::chrono::steady_clock::time_point& iteration_start_time) {

    const std::chrono::steady_clock::time_point iteration_end_time =
            std::chrono::steady_clock::now();
    const std::chrono::milliseconds iteration_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(iteration_end_time -
                                                                  iteration_start_time);

    if (iteration_time < game_iteration_duration)
        std::this_thread::sleep_for(game_iteration_duration - iteration_time);
}


void GameLoop::process_commands_in_waiting_car_selection_phase() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) {

        ClientAction action = game_command.get_client_action();

        if (action == ClientAction::DISCONNECT) {

            handle_disconnection(game_command);
            continue;
        } else if (action == ClientAction::SEND_CAR_SELECTION) {

            handle_car_selection(game_command);
            continue;
        }
    }
}

void GameLoop::process_commands_in_upgrade_shop_phase() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) {

        ClientAction action = game_command.get_client_action();

        if (action == ClientAction::DISCONNECT) {

            handle_disconnection(game_command);
            continue;
        } else if (action == ClientAction::SEND_UPGRADE) {

            handle_upgrade_car(game_command);
            continue;
        }
    }
}

void GameLoop::process_commands_in_countdown_phase() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) {

        ClientAction action = game_command.get_client_action();

        if (action == ClientAction::DISCONNECT) {

            handle_disconnection(game_command);
            continue;
        }
    }
}

void GameLoop::handle_cheat_win_race(const GameCommand& game_command, const uint32_t elapsed_seconds) {
    //finish race to the player to send the leaderboard
    //to the rest finish race with timeout time
    uint32_t player_id = game_command.get_player_id();
    for (auto& [id, player] : players) {
        if (id != player_id) {
            player.finish_race();
            //convert race_timeout to uint32_t seconds
            uint32_t race_timeout = static_cast<uint32_t>(race_timeout_seconds.count());
            player.record_time(race_timeout);
        } else {
            player.finish_race();
            player.record_time(elapsed_seconds);
        }
    }
}

void GameLoop::handle_cheat_win_game(const GameCommand& game_command, const uint32_t elapsed_seconds) {
    //clean the leaderboard vector. Finish the race for all players with timeout time except for the cheater
    //activate the flag to finish the game after leaderboard
    handle_cheat_win_race(game_command, elapsed_seconds);
    leaderboards.clear();
    LeaderboardDTO leaderboard = create_leaderboard();
    cheat_game_winner_activated = true;
}

void GameLoop::handle_cheat_infinite_health(const GameCommand& game_command) {
    uint32_t player_id = game_command.get_player_id();
    auto it = players.find(player_id);
    if (it != players.end()) {
        it->second.activate_infinite_health();
    }
}

void GameLoop::process_commands_in_race_phase(const uint32_t elapsed_seconds) {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) {

        ClientAction action = game_command.get_client_action();

        if (action == ClientAction::DISCONNECT) {

            handle_disconnection(game_command);
            continue;
        } else if (action == ClientAction::SEND_CAR_MOVEMENT) {

            handle_car_movement(game_command);
            continue;
        } else if (action == ClientAction::CHEAT_WIN_RACE) {
            std::cerr << "[" << GAME_LOOP << "] Player " << game_command.get_player_id() << " used cheat: WIN_RACE" << std::endl;
            handle_cheat_win_race(game_command, elapsed_seconds);
            continue;
        } else if (action == ClientAction::CHEAT_WIN_GAME) {
            std::cerr << "[" << GAME_LOOP << "] Player " << game_command.get_player_id() << " used cheat: WIN_GAME" << std::endl;
            handle_cheat_win_game(game_command, elapsed_seconds);
            continue;
        } else if (action == ClientAction::CHEAT_INFINITE_HEALTH) {
            std::cerr << "[" << GAME_LOOP << "] Player " << game_command.get_player_id() << " used cheat: INFINITE_HEALTH" << std::endl;
            handle_cheat_infinite_health(game_command);
            continue;
        }
    }
}

void GameLoop::process_commands_in_leaderboard_phase() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) {

        ClientAction action = game_command.get_client_action();

        if (action == ClientAction::DISCONNECT) {

            handle_disconnection(game_command);
            continue;
        }
    }
}

void GameLoop::process_commands_in_game_finished_phase() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) {

        ClientAction action = game_command.get_client_action();

        if (action == ClientAction::DISCONNECT) {

            handle_disconnection(game_command);
            continue;
        }
    }
}

void GameLoop::process_commands_in_return_to_lobby_phase() {
    GameCommand game_command;
    while (command_queue.try_pop(game_command)) {

        ClientAction action = game_command.get_client_action();

        if (action == ClientAction::DISCONNECT) {

            handle_disconnection(game_command);
            continue;
        }
    }
}


void GameLoop::handle_waiting_car_selection_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    process_commands_in_waiting_car_selection_phase();

    if (all_player_in_game() && players.size() == max_players) {

        std::cerr << "[" << GAME_LOOP << "] Switching to upgrade shop phase." << std::endl;
        current_phase = GamePhase::UPGRADE_SHOP;
        phase_start = std::nullopt;

        GameUpdateDTO game_update(GameLoopAction::START_SHOP, std::monostate{});
        game_monitor.broadcast(game_update);

        set_race(selected_races.races[current_race_index]);
    }

    fixed_sleep(now);
}


void GameLoop::handle_upgrade_shop_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    const auto elapsed = now - *phase_start;

    process_commands_in_upgrade_shop_phase();

    if (elapsed >= upgrade_shop_seconds) {

        std::cerr << "[" << GAME_LOOP << "] Switching to countdown phase." << std::endl;
        current_phase = GamePhase::COUNTDOWN;
        phase_start = std::nullopt;
        update_positions(selected_races.races[current_race_index], 0);

        GameUpdateDTO game_update(GameLoopAction::START_COUNTDOWN, std::monostate{});
        game_monitor.broadcast(game_update);

    }

    fixed_sleep(now);
}


void GameLoop::handle_countdown_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto elapsed_time = now - *phase_start;

    process_commands_in_countdown_phase();

    if (elapsed_time >= countdown_seconds) {

        std::cerr << "[" << GAME_LOOP << "] Switching to in race phase." << std::endl;
        current_phase = GamePhase::IN_RACE;
        phase_start = std::nullopt;

        GameUpdateDTO game_update(GameLoopAction::START_RACE, std::monostate{});
        game_monitor.broadcast(game_update);
    }

    fixed_sleep(now);
}


void GameLoop::handle_in_race_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    const auto elapsed = now - *phase_start;

    double elapsed_seconds = std::chrono::duration<double>(elapsed).count();
    // check_race_times(elapsed_seconds);
    process_commands_in_race_phase(elapsed_seconds);

    update_positions(selected_races.races[current_race_index], elapsed_seconds);

    check_alive_players();


    box2d_world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

    if (elapsed >= race_timeout_seconds || !player_in_race()) {
        phase_start = std::nullopt;

        if (cheat_game_winner_activated) {
            std::cerr << "[" << GAME_LOOP << "] Cheat WIN_GAME activated. Finishing game." << std::endl;
            current_phase = GamePhase::GAME_FINISHED;
            final_leaderboard = create_final_leaderboard();

            GameUpdateDTO game_update(GameLoopAction::GAME_FINISHED, std::monostate{});
            game_monitor.broadcast(game_update);

            GameUpdateDTO l(GameLoopAction::SEND_LEADERBOARD, final_leaderboard);
            game_monitor.broadcast(l);

        } else {
            std::cerr << "[" << GAME_LOOP << "] Switching to leaderboard phase." << std::endl;
            current_phase = GamePhase::LEADERBOARD;
            LeaderboardDTO leaderboard = create_leaderboard();
            (void)leaderboard;

            GameUpdateDTO game_update(GameLoopAction::LEADERBOARD, std::monostate{});
            game_monitor.broadcast(game_update);

            GameUpdateDTO l(GameLoopAction::SEND_LEADERBOARD, leaderboard);
            game_monitor.broadcast(l);
        }
    }

    fixed_sleep(now);
}

void GameLoop::handle_leaderboard_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    const auto elapsed = now - *phase_start;

    process_commands_in_leaderboard_phase();

    if (elapsed >= leaderboard_seconds) {

        phase_start = std::nullopt;
        if (current_race_index + 1 < selected_races.size) {
            phase_start = std::nullopt;
            current_phase = GamePhase::UPGRADE_SHOP;
            current_race_index += 1;

            GameUpdateDTO game_update(GameLoopAction::START_SHOP, std::monostate{});
            game_monitor.broadcast(game_update);

            set_race(selected_races.races[current_race_index]);
        } else {
            std::cout << "[" << GAME_LOOP << "] Switching to game finished phase." << std::endl;
            current_phase = GamePhase::GAME_FINISHED;
            final_leaderboard = create_final_leaderboard();

            GameUpdateDTO game_update(GameLoopAction::GAME_FINISHED, std::monostate{});
            game_monitor.broadcast(game_update);

            GameUpdateDTO l(GameLoopAction::SEND_LEADERBOARD, final_leaderboard);
            game_monitor.broadcast(l);
        }
    }
    fixed_sleep(now);
}


void GameLoop::handle_game_finished_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto elapsed_time = now - *phase_start;

    process_commands_in_game_finished_phase();


    if (elapsed_time >= game_finish_seconds) {
        phase_start = std::nullopt;
        std::cout << "[" << GAME_LOOP << "] Switching to return to lobby phase." << std::endl;
        current_phase = GamePhase::RETURN_TO_LOBBY;

        GameUpdateDTO game_update(GameLoopAction::RETURN_TO_LOBBY, std::monostate{});
        game_monitor.broadcast(game_update);
    }
    fixed_sleep(now);
}

void GameLoop::handle_return_to_lobby() {
    // disconnect all players
    if (!players.empty()) {
        process_commands_in_return_to_lobby_phase();
    }
}


void GameLoop::remove_player(const uint32_t player_id) {

    const auto it = players.find(player_id);

    if (it != players.end())
        players.erase(it);

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << player_id << " has been removed from the Game World."
              << std::endl;
}

bool GameLoop::is_full() const { return players.size() >= max_players; }

bool GameLoop::is_empty() const { return players.empty(); }

uint8_t GameLoop::get_active_players() const { return static_cast<uint8_t>(players.size()); }

uint32_t GameLoop::get_next_player_id() const {

    if (players.size() < 1)
        return 0;

    return players.begin()->first;
}

bool GameLoop::has_started() const { return current_phase != GamePhase::WAITING_CAR_SELECTION; }

bool GameLoop::has_player(const uint32_t player_id) const {

    const auto it = players.find(player_id);

    if (it != players.end())
        return true;
    
    return false;;   
}



void GameLoop::add_player(const uint32_t player_id, const std::string& username) {

    players.try_emplace(player_id, player_id, username);

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << player_id << " has joined the game. | Username: " << username
              << std::endl
              << std::endl;
}

CarConfig GameLoop::get_car_config(const ServerConfig& server_config, const CarDTO& car_dto) {

    std::string car_model = car_dto.model;

    if (car_model == server_config.physics.compact.model)
        return server_config.physics.compact;
    else if (car_model == server_config.physics.sport.model)
        return server_config.physics.sport;
    else if (car_model == server_config.physics.convertible.model)
        return server_config.physics.convertible;
    else if (car_model == server_config.physics.sedan.model)
        return server_config.physics.sedan;
    else if (car_model == server_config.physics.jeep.model)
        return server_config.physics.jeep;
    else if (car_model == server_config.physics.rally.model)
        return server_config.physics.rally;
    else if (car_model == server_config.physics.truck.model)
        return server_config.physics.truck;
    else
        return server_config.physics.compact;
}


void GameLoop::handle_car_selection(const GameCommand& game_command) {

    if (auto car = game_command.get_data<CarDTO>()) {
        uint32_t player_id = game_command.get_player_id();
        CarDTO car_dto = car->get();
        auto it = players.find(player_id);
        if (it != players.end()) {

            CarConfig car_config = get_car_config(config, car_dto);
            it->second.create_car(car_dto, car_config, box2d_world);
            it->second.set_in_game();
        }
    }
}


void GameLoop::handle_car_movement(const GameCommand& game_command) {

    if (auto movement = game_command.get_data<MovementDTO>()) {

        uint32_t player_id = game_command.get_player_id();
        auto it = players.find(player_id);
        if (it != players.end())
            it->second.handle_car_movement(movement->get());
    }
}

void GameLoop::handle_disconnection(const GameCommand& game_command) {

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << game_command.get_player_id() << " has disconnected."
              << std::endl;

    GameUpdateDTO return_to_lobby(GameLoopAction::RETURN_TO_LOBBY, std::monostate{});
    game_monitor.send_update_to_player(return_to_lobby, game_command.get_player_id());

    std::cerr << "Sent 'Return to Lobby' action to the player with ID: " << game_command.get_player_id() << std::endl;

    game_monitor.unregister_player(game_command.get_player_id());
}

void GameLoop::handle_upgrade_car(const GameCommand& game_command) {
    auto upgrade = game_command.get_data<BuyUpgradeDTO>();
    if (upgrade) {
        uint32_t player_id = game_command.get_player_id();
        auto it = players.find(player_id);
        if (it != players.end()) {
            it->second.set_upgrade(upgrade->get(), config.game.upgrades);
        }
    }
}

void GameLoop::update_positions(const RaceDTO& race, uint32_t elapsed_seconds) {

    WorldSnapshotDTO world_snapshot;

    for (auto& [player_id, player]: players) {

        player.update_car_position(race, elapsed_seconds);
        PlayerDTO player_dto = player.get_info();
        world_snapshot.add_car_position(player_dto, player.get_car_position());
    }

    // Update NPC positions
    for (auto& npc: npcs) {
        npc.update();
        world_snapshot.add_npc_position(npc.get_position());
    }

    GameUpdateDTO update(GameLoopAction::SEND_UPDATE_POSITIONS, world_snapshot);

    game_monitor.broadcast(update);
}

void GameLoop::send_race(const RaceDTO& race) {

    GameUpdateDTO race_update(GameLoopAction::SEND_RACE, race);
    game_monitor.broadcast(race_update);
}


bool GameLoop::all_player_in_game() const {

    return std::all_of(players.begin(), players.end(),
                       [](const auto& p) { return p.second.is_in_game(); });
}

bool GameLoop::player_in_race() const {

    return std::any_of(players.begin(), players.end(),
                       [](const auto& p) { return p.second.is_in_race(); });
}

void GameLoop::map_configuration(const std::string& city_name) {
    city_loader.load_city(city_name);
    load_npcs(city_name);
}

void GameLoop::load_npcs(const std::string& city_name) {
    npcs.clear();

    std::string normalized_city = city_name;
    std::transform(normalized_city.begin(), normalized_city.end(), normalized_city.begin(),
                   ::tolower);
    std::replace(normalized_city.begin(), normalized_city.end(), ' ', '_');

    std::string npc_file = "server/world/npcs/npcs_" + normalized_city + ".json";

    try {
        std::ifstream file(npc_file);
        if (!file) {
            std::cout << "[GAME_LOOP] NPC file not found: " << npc_file << std::endl;
            return;
        }

        nlohmann::json json_data;
        file >> json_data;

        if (!json_data.is_array()) {
            std::cout << "[GAME_LOOP] Invalid NPC file format" << std::endl;
            return;
        }

        uint32_t npc_id = 0;
        constexpr float NPC_SPEED = 1.5f;  // Slow speed in pixels per update

        for (const auto& polygon: json_data) {
            if (polygon.is_array() && polygon.size() >= 3) {
                std::vector<std::vector<int>> route;
                for (const auto& point: polygon) {
                    if (point.is_array() && point.size() >= 2) {
                        route.push_back({point[0].get<int>(), point[1].get<int>()});
                    }
                }
                npcs.emplace_back(npc_id++, route, NPC_SPEED);
            }
        }

        std::cout << "[GAME_LOOP] Loaded " << npcs.size() << " NPCs from " << npc_file << std::endl;

    } catch (const std::exception& e) {
        std::cout << "[GAME_LOOP] Error loading NPCs: " << e.what() << std::endl;
    }
}

void GameLoop::set_race(const RaceDTO& race) {

    race_active = true;

    send_race(race);

    map_configuration(race.city);

    for (auto& [player_id, player]: players) {

        player.set_position_for_race(race.checkpoints);
        player.start_race();
    }
}

void GameLoop::finish_race(uint32_t timeout_time) {

    for (auto& [player_id, player]: players) {

        if (!player.has_finished_race()) {

            player.finish_race();
            player.record_time(timeout_time);
        }
    }
}

void GameLoop::check_alive_players() {

    uint32_t timeout = static_cast<uint32_t>(race_timeout_seconds.count());

    for (auto& [player_id, player]: players) {

        if (player.is_dead() && player.is_in_race()) {
            std::cout << "Player with ID: " << player_id << " has died during the race."
                      << std::endl;
            player.finish_race();
            player.record_time(timeout);
        }
    }
}

void GameLoop::check_race_times(uint32_t seconds) {

    for (auto& [player_id, player]: players) {

        if (player.has_finished_race() and !player.is_dead()) {

            std::cout << "Player with ID: " << player_id << " has finished the race in " << seconds
                      << " seconds." << std::endl;
            player.record_time(seconds);
        }
    }
}

LeaderboardDTO GameLoop::create_final_leaderboard() {

    std::vector<PlayerRaceStatsDTO> final_stats;

    for (const auto& leaderboard: leaderboards) {
        for (const auto& stats_race: leaderboard.players) {
            auto it = std::find_if(final_stats.begin(), final_stats.end(),
                                   [&stats_race](const PlayerRaceStatsDTO& stats_final) {
                                       return stats_final.client_id == stats_race.client_id;
                                   });
            if (it != final_stats.end()) {

                it->points += stats_race.points;
                it->total_time += stats_race.total_time;
            } else {

                final_stats.emplace_back(stats_race);
            }
        }
    }

    std::sort(final_stats.begin(), final_stats.end(),
              [](const PlayerRaceStatsDTO& a, const PlayerRaceStatsDTO& b) {
                  if (a.points == b.points)
                      return a.total_time < b.total_time;
                  return a.points > b.points;
              });

    uint8_t position = 1;
    for (size_t i = 0; i < final_stats.size(); ++i) {

        final_stats[i].position = position;
        position++;
    }

    std::cout << "\nFinal Leaderboard: " << std::endl;
    for (const auto& stats: final_stats) {
        std::cout << "Position: " << static_cast<int>(stats.position)
                  << " | Player ID: " << stats.client_id << " | Username: " << stats.username
                  << " | Total Time: " << stats.total_time << " seconds"
                  << " | Total Points: " << static_cast<int>(stats.points) << std::endl;
    }

    return LeaderboardDTO("Final Leaderboard", final_stats);
}

LeaderboardDTO GameLoop::create_leaderboard() {

    std::vector<PlayerRaceStatsDTO> race_stats;

    for (const auto& [player_id, player]: players) {

        PlayerRaceStatsDTO stats = player.get_race_stats();
        race_stats.push_back(stats);
    }

    std::sort(race_stats.begin(), race_stats.end(),
              [](const PlayerRaceStatsDTO& a, const PlayerRaceStatsDTO& b) {
                  return a.total_time < b.total_time;
              });

    uint8_t points = 8;
    uint8_t position = 1;
    for (size_t i = 0; i < race_stats.size(); ++i) {
        race_stats[i].position = position;
        position++;
        if (std::chrono::seconds(race_stats[i].total_time) >= race_timeout_seconds) {
            race_stats[i].points = 0;
            continue;  // skip players who did not finish the race
        }
        race_stats[i].points = points;
        points--;
    }

    std::string race_name = selected_races.races[current_race_index].name;

    LeaderboardDTO actual(race_name, race_stats);

    leaderboards.push_back(actual);

    // cout debug of leaderboard
    std::cout << "\nLeaderboard for race: " << race_name << std::endl;
    for (const auto& stats: actual.players) {
        std::cout << "Position: " << static_cast<int>(stats.position)
                  << " | Player ID: " << stats.client_id << " | Username: " << stats.username
                  << " | Time: " << stats.total_time << " seconds"
                  << " | Points: " << static_cast<int>(stats.points) << std::endl;
    }

    return actual;
}


void GameLoop::run() {

    try {

        while (should_keep_running()) {

            if (!phase_start.has_value())
                phase_start = std::chrono::steady_clock::now();

            switch (current_phase) {

                case (GamePhase::WAITING_CAR_SELECTION):
                    handle_waiting_car_selection_phase();
                    break;
                case (GamePhase::UPGRADE_SHOP):
                    handle_upgrade_shop_phase();
                    break;
                case (GamePhase::COUNTDOWN):
                    handle_countdown_phase();
                    break;
                case (GamePhase::IN_RACE):
                    handle_in_race_phase();
                    break;
                case (GamePhase::LEADERBOARD):
                    handle_leaderboard_phase();
                    break;
                case (GamePhase::GAME_FINISHED):
                    handle_game_finished_phase();
                    break;
                case (GamePhase::RETURN_TO_LOBBY):
                    handle_return_to_lobby();
                    break;
            }
        }
    } catch (const ClosedQueue&) {}
}


void GameLoop::stop() {

    if (_keep_running)
        _keep_running = false;

    try {

        players.clear();
    } catch (const std::exception&) {}
}


GameLoop::~GameLoop() {

    try {

        GameLoop::stop();
    } catch (const std::exception& e) {

        std::cerr << "[" << GAME_LOOP << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << e.what();
    }
}
