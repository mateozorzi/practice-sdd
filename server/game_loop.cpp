#include "game_loop.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include "../common/checkpoint_dto.h"
#include "../common/client_event_dto.h"
#include "../common/constants.h"
#include "../common/game_update_dto.h"
#include "../common/leaderboard_dto.h"

#include "world_creation_exception.h"

#define RACE_TIMEOUT_SEG 600  // 10 minutes


GameLoop::GameLoop(GameMonitor& game_monitor, Queue<GameCommand>& command_queue,
                   const uint8_t max_players, const ServerConfig& config,
                   RaceCatalogueDTO& selected):
        config(config),
        max_players(max_players),
        game_iteration_duration(1000 / config.game.game_fps),
        game_monitor(game_monitor),
        command_queue(command_queue),
        box2d_world(b2Vec2(0.0f, 0.0f)),
        city_loader(box2d_world),
        collision_listener(players),
        selected_races(selected),
        race_active(false) {

    box2d_world.SetContactListener(&collision_listener);

    command_handlers = {
            {ClientAction::SEND_CAR_SELECTION,
             [this](const GameCommand& game_command) { handle_car_selection(game_command); }},
            {ClientAction::SEND_CAR_MOVEMENT,
             [this](const GameCommand& game_command) { handle_car_movement(game_command); }},
            {ClientAction::DISCONNECT,
             [this](const GameCommand& game_command) { handle_disconnection(game_command); }},
            {ClientAction::SEND_UPGRADE,
             [this](const GameCommand& game_command) { handle_upgrade_car(game_command); }}};
}


void GameLoop::fixed_sleep(const std::chrono::steady_clock::time_point& iteration_start_time) {

    const std::chrono::steady_clock::time_point iteration_end_time = std::chrono::steady_clock::now();
    const std::chrono::milliseconds iteration_time = std::chrono::duration_cast<std::chrono::milliseconds>(iteration_end_time - iteration_start_time);

    if (iteration_time < game_iteration_duration)
        std::this_thread::sleep_for(game_iteration_duration - iteration_time);
}

/*


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


void GameLoop::handle_waiting_car_selection_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    process_commands_in_waiting_car_selection_phase();

    if (all_player_in_game()) {

        current_phase = GamePhase::UPGRADE_SHOP;
    }

    fixed_sleep(now);
}



void GameLoop::handle_upgrade_shop_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    const std::chrono::steady_clock::time_point elapsed = now - phase_start;

    process_commands_in_upgrade_shop_phase();

    if (elapsed_time >= upgrade_shop_seconds)
        current_phase = GamePhase::COUNTDOWN;

    fixed_sleep(now);
}



void GameLoop::handle_countdown_phase(const uint8_t current_race_index) {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto elapsed_time = now - phase_start;

    map_configuration(selected_races.races[current_race_index].city);

    process_commands_in_waiting_car_selection_phase();

    //update_positions(selected_races.races[current_race_index]);

    if (elapsed_time >= countdown_seconds) {

        start_race(selected_races.races[current_race_index]);
        current_phase = GamePhase::IN_RACE;
    }

    fixed_sleep(now);
}


void GameLoop::handle_in_race_phase(const uint8_t current_race_index) {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    const std::chrono::steady_clock::time_point elapsed = now - phase_start;

    process_commands_in_race_phase();

    update_positions(selected_races.races[current_race_index]);

    //check_race_times(seconds);

    box2d_world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

    if (elapsed_time >= race_timeout_minutes)
        current_phase = GamePhase::UPGRADE_SHOP;

    fixed_sleep(now);
}


void GameLoop::handle_game_finished_phase() {

    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto elapsed_time = now - phase_start;

    process_commands_in_game_finished_phase();

    if (elapsed_time >= final_leaderboard_seconds)
        //return to lobby here!

    fixed_sleep(now);
}

*/



void GameLoop::remove_player(const uint32_t client_id) {

    const auto it = players.find(client_id);

    if (it != players.end())
        players.erase(it);

    game_monitor.unregister_client(client_id);

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << client_id << " has been removed from the Game World."
              << std::endl;
}

bool GameLoop::is_full() const { return players.size() >= max_players; }

bool GameLoop::is_empty() const { return players.empty(); }

uint8_t GameLoop::get_active_players() const { return static_cast<uint8_t>(players.size()); }


void GameLoop::add_player(const uint32_t client_id, const std::string& username) {

    players.try_emplace(client_id, client_id, username);

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << client_id << " has joined the game. | Username: " << username
              << std::endl
              << std::endl;
}

void GameLoop::process_commands() {

    GameCommand game_command;

    while (command_queue.try_pop(game_command)) handle_command(game_command);
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
        uint32_t client_id = game_command.get_client_id();
        CarDTO car_dto = car->get();
        auto it = players.find(client_id);
        if (it != players.end()) {

            CarConfig car_config = get_car_config(config, car_dto);
            it->second.create_car(car_dto, car_config, box2d_world);
            it->second.set_in_game();
        }
    }
}


void GameLoop::handle_car_movement(const GameCommand& game_command) {

    if (auto movement = game_command.get_data<MovementDTO>()) {

        uint32_t client_id = game_command.get_client_id();
        auto it = players.find(client_id);
        if (it != players.end())
            it->second.handle_car_movement(movement->get());
    }
}

void GameLoop::handle_disconnection(const GameCommand& game_command) {

    std::cerr << "[" << GAME_LOOP << "] "
              << "Player with ID: " << game_command.get_client_id() << " has disconnected."
              << std::endl;

    remove_player(game_command.get_client_id());
}

void GameLoop::handle_upgrade_car(const GameCommand& game_command) {
    auto upgrade = game_command.get_data<BuyUpgradeDTO>();
    if (upgrade) {
        uint32_t client_id = game_command.get_client_id();
        auto it = players.find(client_id);
        if (it != players.end()) {
            it->second.set_upgrade(upgrade->get(), config.game.upgrades);
        }
    }
}

void GameLoop::handle_command(const GameCommand& game_command) {

    const auto it = command_handlers.find(game_command.get_client_action());

    if (it != command_handlers.end())
        it->second(game_command);
}

void GameLoop::update_positions(const RaceDTO& race) {

    WorldSnapshotDTO world_snapshot;

    for (auto& [client_id, player]: players) {
        player.update_car_position(race);
        // CarDTO car = player.get_car();
        PlayerDTO player_dto = player.get_info();
        world_snapshot.add_car_position(player_dto, player.get_car_position());
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

void GameLoop::map_configuration(const std::string& city_name) { city_loader.load_city(city_name); }

void GameLoop::shop_phase() {
    // future implementation
    GameUpdateDTO shop_update(GameLoopAction::START_SHOP, std::monostate{});
    game_monitor.broadcast(shop_update);
    // std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    GameUpdateDTO shop_close(GameLoopAction::CLOSE_SHOP, std::monostate{});
    game_monitor.broadcast(shop_close);
}

void GameLoop::start_race(const RaceDTO& race) {

    race_active = true;

    send_race(race);

    map_configuration(race.city);

    for (auto& [client_id, player]: players) {
        // setting the positions of the players at the start line (first checkpoint)
        player.set_position_for_race(race.checkpoints);
        player.start_race();
    }

    // the race start, countdown 3,2,1, go!
    // server sends countdown to players
    // sleep 3 seconds?
    // std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}

void GameLoop::finish_race(uint32_t timeout_time) {
    for (auto& [client_id, player]: players) {
        if (!player.has_finished_race()) {
            player.finish_race();
            player.record_time(timeout_time);
        }
    }
}

void GameLoop::check_alive_players(uint32_t race_timeout_ms) {
    // check if any player is dead
    for (auto& [client_id, player]: players) {
        if (player.is_dead() && player.is_in_race()) {
            std::cout << "Player with ID: " << client_id << " has died during the race."
                      << std::endl;
            player.finish_race();
            player.record_time(race_timeout_ms);
        }
    }
}

void GameLoop::check_race_times(uint32_t seconds) {
    // check if any player has finished the race
    for (auto& [client_id, player]: players) {
        if (player.has_finished_race() and !player.is_dead()) {
            std::cout << "Player with ID: " << client_id << " has finished the race in " << seconds
                      << " seconds." << std::endl;
            player.record_time(seconds);
        }
    }
}

void GameLoop::create_final_leaderboard() {
    std::vector<PlayerRaceStatsDTO> final_stats;

    // recoplite form every leaderboard the stats of every player
    // if PlayerRaceStatsDTO for a client_id in final stat dont exist, create it and add to
    // final_stats if exist, sum points and total_time_ms in final stats
    for (const auto& leaderboard: leaderboards) {
        for (const auto& stats_race: leaderboard.players) {
            auto it = std::find_if(final_stats.begin(), final_stats.end(),
                                   [&stats_race](const PlayerRaceStatsDTO& stats_final) {
                                       return stats_final.client_id == stats_race.client_id;
                                   });
            if (it != final_stats.end()) {
                // found, sum points and total_time
                it->points += stats_race.points;
                it->total_time += stats_race.total_time;
            } else {
                // not found, create new PlayerRaceStatsDTO and add to final_stats
                final_stats.emplace_back(stats_race);
            }
        }
    }

    // sort finals_stats by points desc, if tie, by total_time_ms asc
    std::sort(final_stats.begin(), final_stats.end(),
              [](const PlayerRaceStatsDTO& a, const PlayerRaceStatsDTO& b) {
                  if (a.points == b.points) {
                      return a.total_time < b.total_time;  // Ascending order for total_time
                  }
                  return a.points > b.points;  // Descending order for points
              });

    // add positions to final_stats
    uint8_t position = 1;
    for (size_t i = 0; i < final_stats.size(); ++i) {
        final_stats[i].position = position;
        position++;
    }

    // debug cout final leaderboard
    std::cout << "\nFinal Leaderboard: " << std::endl;
    for (const auto& stats: final_stats) {
        std::cout << "Position: " << static_cast<int>(stats.position)
                  << " | Player ID: " << stats.client_id << " | Username: " << stats.username
                  << " | Total Time: " << stats.total_time << " seconds"
                  << " | Total Points: " << static_cast<int>(stats.points) << std::endl;
    }

    final_leaderboard = LeaderboardDTO("Final Leaderboard", final_stats);

    // future implementation
    // GameUpdateDTO update(GameLoopAction::SEND_LEADERBOARD, final_leaderboard);
    // game_monitor.broadcast(update);
}

void GameLoop::create_leaderboard(uint8_t current_race_index) {
    // create leaderboard
    std::vector<PlayerRaceStatsDTO> race_stats;

    // for every player, create a PlayerRaceStatsDTO
    for (const auto& [client_id, player]: players) {
        PlayerRaceStatsDTO stats = player.get_race_stats();
        race_stats.push_back(stats);
    }

    // sort by race_time,
    std::sort(race_stats.begin(), race_stats.end(),
              [](const PlayerRaceStatsDTO& a, const PlayerRaceStatsDTO& b) {
                  return a.total_time < b.total_time;
              });
    // and assign points
    // First place: 8 points, second: 7, third: 6, fourth: 5, fifth: 4, sixth: 3, seventh: 2,
    // eighth: 1
    uint8_t points = 8;
    uint8_t position = 1;
    for (size_t i = 0; i < race_stats.size(); ++i) {
        race_stats[i].position = position;
        position++;
        if (race_stats[i].total_time >= RACE_TIMEOUT_SEG) {
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

    // future implementation
    // GameUpdateDTO update(GameLoopAction::SEND_LEADERBOARD, actual);

    // game_monitor.broadcast(update);
}

enum GamePhase { SHOP, START_RACE, RACE, RACE_LEADERBOARD, FINAL_LEADERBOARD, FINISH_GAME };

void GameLoop::run() {

    try {

        uint8_t current_race_index = 0;

        static constexpr auto RACE_TIMEOUT = std::chrono::minutes(10);
        std::chrono::steady_clock::time_point race_start_time;

        GamePhase phase = START_RACE;

        while (should_keep_running()) {
            
            /*if (!phase_start.has_value())
                phase_start = std::chrono::steady_clock::now();

            switch (current_phase) {

                case (GamePhase::WAITING_CAR_SELECTION):
                    handle_waiting_car_selection_phase();
                    break;
                case (GamePhase::UPGRADE_SHOP):
                    handle_upgrade_shop_phase();
                    break;
                case (GamePhase::COUNTDOWN):
                    handle_countdown_phase(current_race_index);
                    break;
                case (GamePhase::IN_RACE):
                    handle_in_race_phase(current_race_index);
                    break;
                case (GamePhase::GAME_FINISHED):
                    handle_game_finished_phase();
                    break;
            }*/

            const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            
            process_commands();
            
            if (all_player_in_game()) {
                
                switch (phase){
                    case SHOP:
                        shop_phase();
                        phase = START_RACE;
                        break;


                    case START_RACE:
                    // start races
                        if (!race_active && current_race_index < selected_races.size) {
                            start_race(selected_races.races[current_race_index]);
                            race_start_time = std::chrono::steady_clock::now();
                            phase = RACE;
                        }
                        break;

                    case RACE:
                        if (player_in_race()) {
                        auto current_time = std::chrono::steady_clock::now();

                        auto elapsed = current_time - race_start_time;
                        uint32_t seconds =
                                std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

                        if (elapsed >= RACE_TIMEOUT) {
                            // race finish by timeout
                            // finish players who are still racing
                            finish_race(seconds);
                            continue;
                        }

                        check_alive_players(static_cast<uint32_t>(RACE_TIMEOUT_SEG));
                        update_positions(selected_races.races[current_race_index]);
                        check_race_times(seconds);
                        box2d_world.Step(GAME_STEP, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
                        fixed_sleep(now);
                        continue;
                        }else{
                            // if no player racing
                            phase = RACE_LEADERBOARD;
                        }
                        break;
                    
                    case RACE_LEADERBOARD:
                        // if no player racing, race is finished -> show results and start next one.
                        if (race_active) {
                            race_active = false;

                            // show player time
                            for (const auto& [client_id, player]: players) {
                                std::cout << "Player with ID: " << client_id
                                            << " recorded time: " << player.get_recorded_time() << " seconds."
                                            << std::endl;
                            }

                            // create leaderboard and reset race_time to players
                            create_leaderboard(current_race_index);
                            // send FINISH_RACE

                            // server send leaderboard to players, actual_race
                            // push_back actual race into leaderboards, update final_leaderboard and reset
                            // actual_race sleep to show leaderboard for some seconds, 5 seconds?
                            current_race_index++;

                            if (current_race_index >= selected_races.size) {
                                // all races finished
                                // create and send final leaderboard
                                phase = FINAL_LEADERBOARD;
                            }else{
                                phase = SHOP;
                            }
                        }
                        break;

                    case FINAL_LEADERBOARD:
                        create_final_leaderboard();
                        // sleep to show final leaderboard for some seconds, 10 seconds?
                        phase = FINISH_GAME;
                        break;

                    case FINISH_GAME:
                        // end game loop
                        this->stop();
                        break;
                }

                fixed_sleep(now);
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
