#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <thread>

#include <SDL2/SDL.h>
#include <gtest/gtest.h>

#include "../../client/camera.h"
#include "../../client/client_constants.h"
#include "../../client/client_protocol.h"
#include "../../client/game.h"
#include "../../client/input_manager.h"
#include "../../client/power_up_view.h"
#include "../../client/sdl_manager.h"
#include "../../common/socket.h"
#include "../../server/server_protocol.h"

// Tests of the SDL side of the client. The window is drawn offscreen (SDL "offscreen" video
// driver) and the sound goes to the "dummy" audio driver. The client loads its images, fonts
// and sounds from the installation folder, so these tests are skipped when it is missing.

namespace {

std::atomic<int> next_port{29500};

const char* ASSETS_FOLDER = "/var/nfs/assets";

std::string read_file(const std::string& relative_path) {
    std::ifstream file(std::string(NFS_SOURCE_DIR) + "/" + relative_path);
    std::stringstream content;
    content << file.rdbuf();
    return content.str();
}

std::string bridges(const std::string& city) {
    return read_file("server/world/object_rendering/bridges_" + city + ".json");
}

std::string decorations(const std::string& city) {
    return read_file("server/world/object_rendering/decorations_" + city + ".json");
}

void push_key(SDL_EventType type, SDL_Keycode key) {
    SDL_Event event{};
    event.type = type;
    event.key.keysym.sym = key;
    SDL_PushEvent(&event);
}

void push_click(int x, int y) {
    SDL_Event event{};
    event.type = SDL_MOUSEMOTION;
    event.motion.x = x;
    event.motion.y = y;
    SDL_PushEvent(&event);

    event = SDL_Event{};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = x;
    event.button.y = y;
    SDL_PushEvent(&event);

    event.type = SDL_MOUSEBUTTONUP;
    SDL_PushEvent(&event);
}

PlayerInfo player_at(uint32_t id, int x, int y) {
    PlayerInfo player;
    player.id = id;
    player.x = x;
    player.y = y;
    player.angle = 45;
    player.life = 80;
    player.max_life = 100;
    player.model = "Jeep";
    player.username = "player" + std::to_string(id);
    player.checkpoint =
            CheckpointDTO(2, 'H', static_cast<uint16_t>(x + 200), static_cast<uint16_t>(y), 66, 15);
    player.current_lap = 1;
    player.race_lap = 2;
    return player;
}

class SdlTest: public ::testing::Test {
protected:
    void SetUp() override {
        if (!std::filesystem::exists(ASSETS_FOLDER))
            GTEST_SKIP() << "the client assets are not installed in " << ASSETS_FOLDER;
        setenv("SDL_VIDEODRIVER", "offscreen", 1);
        setenv("SDL_AUDIODRIVER", "dummy", 1);
    }
};

// Snapshot of the render and sound milestone Background: the own player (id 1) next to the
// power-ups 1 Nitro, 2 Repair and 3 Shield, all active
LocalInfo power_up_state() {
    LocalInfo state{1, {}, player_at(1, 200, 100), {}};
    state.power_ups = {PowerUpDTO(1, PowerUpType::Nitro, 100, 100, true),
                       PowerUpDTO(2, PowerUpType::Repair, 200, 100, true),
                       PowerUpDTO(3, PowerUpType::Shield, 300, 100, true)};
    return state;
}

std::vector<uint8_t> drawn_ids(const LocalInfo& state) {
    const std::vector<PowerUpSprite> sprites = power_ups_to_draw(state);
    std::vector<uint8_t> ids(sprites.size());
    std::transform(sprites.begin(), sprites.end(), ids.begin(),
                   [](const PowerUpSprite& sprite) { return sprite.id; });
    return ids;
}

bool intersect(const EffectBadge& a, const EffectBadge& b) {
    return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
}

// Client protocol connected to a server protocol through a localhost socket
class ConnectedSdlTest: public SdlTest {
protected:
    std::string port = std::to_string(next_port++);
    Socket acceptor{port.c_str()};
    Socket client_socket{"localhost", port.c_str()};
    Socket server_socket = acceptor.accept();
    ClientProtocol client{client_socket};
    ServerProtocol server{server_socket};
};

}  // namespace


// Camera

TEST(CameraTest, CentersTheViewOnThePlayer) {
    Camera camera(500, 300, 5000, 5000);

    camera.update(1000, 2000);

    SDL2pp::Rect view = camera.get_view_rect();
    EXPECT_EQ(view.x, 1000 - 250 + 25);
    EXPECT_EQ(view.y, 2000 - 150 + 25);
    EXPECT_EQ(view.w, 500);
    EXPECT_EQ(view.h, 300);
    EXPECT_EQ(camera.get_center(), SDL2pp::Point(view.x + 250, view.y + 150));
}

TEST(CameraTest, StaysInsideTheWorld) {
    Camera camera(500, 300, 5000, 5000);

    camera.update(0, 0);
    EXPECT_EQ(camera.get_view_rect().x, 0);
    EXPECT_EQ(camera.get_view_rect().y, 0);

    camera.update(5000, 5000);
    EXPECT_EQ(camera.get_view_rect().x, 4500);
    EXPECT_EQ(camera.get_view_rect().y, 4700);
}

TEST(CameraTest, KnowsWhatIsOnScreen) {
    Camera camera(500, 300, 5000, 5000);
    camera.update(1000, 1000);

    EXPECT_TRUE(camera.on_screen(SDL2pp::Rect(1000, 1000, 10, 10)));
    EXPECT_FALSE(camera.on_screen(SDL2pp::Rect(3000, 3000, 10, 10)));
}


// SDLManager: every screen is drawn without errors

TEST_F(SdlTest, DrawsTheWaitingAndCountdownScreens) {
    SDLManager sdl;
    LocalInfo state{1, {}, player_at(1, 1000, 1000), {}};

    sdl.render_waiting();
    sdl.load_city("Liberty City", bridges("liberty_city"), decorations("liberty_city"));
    for (uint8_t seconds: {3, 2, 1, 0}) {
        state.countdown_seconds = seconds;
        sdl.render_countdown(state);
    }
}

TEST_F(SdlTest, DrawsTheRaceInEveryCity) {
    SDLManager sdl;
    for (const auto& [city, file]:
         {std::pair<std::string, std::string>{"Liberty City", "liberty_city"},
          {"San Andreas", "san_andreas"},
          {"Vice City", "vice_city"}}) {
        sdl.load_city(city, bridges(file), decorations(file));

        LocalInfo state{1, {}, player_at(1, 1500, 1500), {NpcDTO(1, 1550, 1500, 90)}};
        state.players[2] = player_at(2, 1600, 1550);
        state.players[3] = player_at(3, 1450, 1450);
        state.players[3].layer = CarLayer::ON_BRIDGE;
        state.players[4] = player_at(4, 4000, 4000);  // off screen
        state.player.isMoving = true;

        for (int frame = 0; frame < 3; ++frame) sdl.render(state);
    }
}

TEST_F(SdlTest, DrawsCrashesExplosionsAndTheEndOfTheRace) {
    SDLManager sdl;
    sdl.load_city("Vice City", bridges("vice_city"), decorations("vice_city"));
    sdl.set_sound_info({1, 2});
    LocalInfo state{1, {}, player_at(1, 2000, 2000), {}};
    state.players[2] = player_at(2, 2050, 2000);

    state.player.crashed = true;
    state.players[2].crashed = true;
    sdl.render(state);
    EXPECT_FALSE(state.player.crashed);
    EXPECT_FALSE(state.players[2].crashed);

    // Other player explodes, then the player: the explosion is shown once
    state.players[2].died = true;
    sdl.render(state);
    EXPECT_TRUE(state.players[2].explosion_shown);
    state.player.died = true;
    state.player.life = 0;
    sdl.render(state);
    EXPECT_TRUE(state.player.explosion_shown);
    for (int frame = 0; frame < 40; ++frame) sdl.render(state);  // the animations end

    state.player.died = false;
    state.player.life = 50;
    state.player.finish_race = true;
    state.player.layer = CarLayer::ON_BRIDGE;
    sdl.render(state);
    EXPECT_FALSE(state.player.explosion_shown);
}

TEST_F(SdlTest, InvalidCityDataIsIgnored) {
    SDLManager sdl;

    testing::internal::CaptureStderr();
    sdl.load_city("Liberty City", "not json", "");
    EXPECT_NE(testing::internal::GetCapturedStderr().find("Error parsing JSON"), std::string::npos);

    LocalInfo state{1, {}, player_at(1, 1000, 1000), {}};
    sdl.render(state);
}

TEST_F(SdlTest, UpgradeShopButtons) {
    SDLManager sdl;
    sdl.load_city("San Andreas", bridges("san_andreas"), decorations("san_andreas"));

    EXPECT_FALSE(sdl.is_shop_open());
    sdl.render_shop(0, 10, 4, 5);  // closed: nothing is drawn
    sdl.toggle_shop();
    EXPECT_TRUE(sdl.is_shop_open());

    EXPECT_EQ(sdl.interact_button(960, 387, 0), std::optional<ButtonType>(Health));
    EXPECT_EQ(sdl.interact_button(960, 528, 0), std::optional<ButtonType>(Speed));
    EXPECT_EQ(sdl.interact_button(960, 669, 0), std::optional<ButtonType>(Acceleration));
    sdl.render_shop(0, 10, 3, 5);  // with the last button hovered
    EXPECT_EQ(sdl.interact_button(960, 810, 0), std::optional<ButtonType>(Handling));
    EXPECT_EQ(sdl.interact_button(10, 10, 0), std::nullopt);
    sdl.render_shop(0, 10, 0, 0);

    sdl.close_shop();
    EXPECT_FALSE(sdl.is_shop_open());
}

TEST_F(SdlTest, Leaderboards) {
    SDLManager sdl;
    sdl.load_city("Liberty City", bridges("liberty_city"), decorations("liberty_city"));
    LeaderboardDTO leaderboard("park day", {PlayerRaceStatsDTO(1, "mateo", 1, 8, 65),
                                            PlayerRaceStatsDTO(2, "ana", 2, 7, 3700)});

    EXPECT_FALSE(sdl.is_leaderboard_open());
    sdl.render_leaderboard(leaderboard, "Leaderboard");
    sdl.open_leaderboard();
    EXPECT_TRUE(sdl.is_leaderboard_open());
    sdl.render_leaderboard(leaderboard, "Leaderboard");
    sdl.render_leaderboard(leaderboard, "Final Leaderboard");
    sdl.render_leaderboard(LeaderboardDTO(), "Final Leaderboard");
    sdl.close_leaderboard();
    EXPECT_FALSE(sdl.is_leaderboard_open());
}

TEST_F(SdlTest, PlaysTheSounds) {
    SDLManager sdl;
    sdl.set_sound_info({1, 2, 3});

    sdl.play_background_music();
    sdl.set_music_volume(10);
    sdl.play_upgrade_sound();
    sdl.play_finished_race_sound();
    sdl.stop_background_music();
}


// InputManager: SDL events become commands for the server

TEST_F(ConnectedSdlTest, KeysMoveTheCarOncePerPress) {
    SDLManager sdl;
    InputManager input(sdl, client);

    push_key(SDL_KEYDOWN, SDLK_w);
    push_key(SDL_KEYDOWN, SDLK_w);  // held key: sent once
    push_key(SDL_KEYDOWN, SDLK_a);
    input.handle_movement();
    EXPECT_TRUE(input.is_moving());

    push_key(SDL_KEYUP, SDLK_w);
    push_key(SDL_KEYUP, SDLK_a);
    push_key(SDL_KEYDOWN, SDLK_s);
    push_key(SDL_KEYDOWN, SDLK_d);
    push_key(SDL_KEYUP, SDLK_s);
    push_key(SDL_KEYUP, SDLK_d);
    push_key(SDL_KEYUP, SDLK_x);
    input.handle_movement();
    EXPECT_FALSE(input.is_moving());

    const std::vector<std::pair<Direction, bool>> expected = {
            {Direction::UP, true},    {Direction::LEFT, true},  {Direction::UP, false},
            {Direction::LEFT, false}, {Direction::DOWN, true},  {Direction::RIGHT, true},
            {Direction::DOWN, false}, {Direction::RIGHT, false}};
    for (const auto& [direction, moving]: expected) {
        GameCommandDTO command = server.receive_game_command();
        ASSERT_EQ(command.client_action, ClientAction::SEND_CAR_MOVEMENT);
        EXPECT_EQ(command.get_data<MovementDTO>()->get().direction, direction);
        EXPECT_EQ(command.get_data<MovementDTO>()->get().isMoving, moving);
    }
}

TEST_F(ConnectedSdlTest, NumberKeysAreCheats) {
    SDLManager sdl;
    InputManager input(sdl, client);

    push_key(SDL_KEYDOWN, SDLK_1);
    push_key(SDL_KEYDOWN, SDLK_2);
    push_key(SDL_KEYDOWN, SDLK_3);
    input.handle_movement();

    EXPECT_EQ(server.receive_game_command().client_action, ClientAction::CHEAT_WIN_RACE);
    EXPECT_EQ(server.receive_game_command().client_action, ClientAction::CHEAT_WIN_GAME);
    EXPECT_EQ(server.receive_game_command().client_action, ClientAction::CHEAT_INFINITE_HEALTH);
}

TEST_F(ConnectedSdlTest, ClickingTheShopBuysUpgrades) {
    SDLManager sdl;
    InputManager input(sdl, client);
    sdl.toggle_shop();

    push_click(960, 387);
    push_click(960, 528);
    push_click(960, 669);
    push_click(960, 810);
    push_click(10, 10);  // outside the buttons
    input.handle_buy_events(0);

    for (ButtonType upgrade: {Health, Speed, Acceleration, Handling}) {
        GameCommandDTO command = server.receive_game_command();
        ASSERT_EQ(command.client_action, ClientAction::SEND_UPGRADE);
        EXPECT_EQ(command.get_data<BuyUpgradeDTO>()->get().upgrade, upgrade);
    }
}

TEST_F(ConnectedSdlTest, EscapeLeavesTheGameInEveryPhase) {
    using Handler = std::function<void(InputManager&)>;
    for (const Handler& handle: {Handler([](InputManager& i) { i.handle_waiting(); }),
                                 Handler([](InputManager& i) { i.handle_buy_events(0); }),
                                 Handler([](InputManager& i) { i.handle_countdown(); }),
                                 Handler([](InputManager& i) { i.handle_movement(); }),
                                 Handler([](InputManager& i) { i.handle_leaderboard(); }),
                                 Handler([](InputManager& i) { i.handle_game_finished(); })}) {
        SDLManager sdl;
        InputManager input(sdl, client);
        push_key(SDL_KEYDOWN, SDLK_x);  // other keys are ignored
        push_key(SDL_KEYDOWN, SDLK_ESCAPE);

        handle(input);
        EXPECT_EQ(server.receive_game_command().client_action, ClientAction::DISCONNECT);

        // Once disconnected the events are no longer read
        push_key(SDL_KEYDOWN, SDLK_q);
        handle(input);
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
    }
}

TEST_F(ConnectedSdlTest, ClosingTheWindowStopsReadingEvents) {
    SDLManager sdl;
    SDL_Event quit{};
    quit.type = SDL_QUIT;
    {
        InputManager input(sdl, client);
        for (int phase = 0; phase < 6; ++phase) {
            SDL_PushEvent(&quit);
            push_key(SDL_KEYDOWN, SDLK_ESCAPE);  // not read: the quit comes first
            switch (phase) {
                case 0:
                    input.handle_waiting();
                    break;
                case 1:
                    input.handle_buy_events(0);
                    break;
                case 2:
                    input.handle_countdown();
                    break;
                case 3:
                    input.handle_movement();
                    break;
                case 4:
                    input.handle_leaderboard();
                    break;
                default:
                    input.handle_game_finished();
                    break;
            }
            SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        }
    }

    // The player never pressed escape: the destructor tells the server the player left
    EXPECT_EQ(server.receive_game_command().client_action, ClientAction::DISCONNECT);
}


// Game: the whole client game against a scripted server

TEST_F(ConnectedSdlTest, PlaysAGameUntilTheServerSendsItBackToTheLobby) {
    const auto wait = []() { std::this_thread::sleep_for(std::chrono::milliseconds(150)); };
    RaceDTO race("park day", "Liberty City", {CheckpointDTO(1, 'H', 1700, 2391, 66, 15)}, 1,
                 bridges("liberty_city"), decorations("liberty_city"));
    WorldSnapshotDTO snapshot;
    snapshot.add_car_position(PlayerDTO("mateo", 7, CarDTO("Jeep"), 100, false, 100),
                              CarPositionDTO(7, 1700, 2300, 0, CheckpointDTO(1, 'H', 1700, 2391)));
    snapshot.add_car_position(PlayerDTO("ana", 8, CarDTO("Truck"), 100, false, 100),
                              CarPositionDTO(8, 1750, 2300, 0));
    LeaderboardDTO leaderboard("park day", {PlayerRaceStatsDTO(7, "mateo", 1, 8, 30)});

    std::vector<ClientAction> received;
    std::thread server_thread([&]() {
        const auto send = [&](GameLoopAction action, const auto& data) {
            server.send_game_update(GameUpdateDTO(action, data));
        };
        wait();
        send(GameLoopAction::START_SHOP, uint8_t{10});
        send(GameLoopAction::SEND_RACE, race);
        send(GameLoopAction::SEND_UPGRADES_REMAINING, uint8_t{4});
        send(GameLoopAction::SEND_UPGRADE_SHOP_TIME_REMAINING, uint8_t{5});
        send(GameLoopAction::SEND_UPDATE_POSITIONS, snapshot);
        wait();
        send(GameLoopAction::SEND_UPDATE_POSITIONS, snapshot);
        send(GameLoopAction::START_COUNTDOWN, std::monostate{});
        send(GameLoopAction::COUNTDOWN_TIME, uint8_t{3});
        send(GameLoopAction::SEND_UPDATE_POSITIONS, snapshot);
        wait();
        send(GameLoopAction::START_RACE, std::monostate{});
        send(GameLoopAction::SEND_UPDATE_POSITIONS, snapshot);
        send(GameLoopAction::RACE_TIME, uint32_t{1});
        wait();
        // The player accelerates. Drawing is slow offscreen and the client may still be in an
        // earlier phase, which ignores the key: keep pressing it until the command arrives.
        std::atomic<bool> moving{false};
        std::thread player([&moving]() {
            while (!moving) {
                push_key(SDL_KEYDOWN, SDLK_w);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
        received.push_back(server.receive_game_command().client_action);
        moving = true;
        player.join();
        send(GameLoopAction::LEADERBOARD, std::monostate{});
        send(GameLoopAction::SEND_LEADERBOARD, leaderboard);
        wait();
        send(GameLoopAction::START_SHOP, uint8_t{10});  // next race
        wait();
        send(GameLoopAction::START_COUNTDOWN, std::monostate{});
        wait();
        send(GameLoopAction::START_RACE, std::monostate{});
        wait();
        send(GameLoopAction::GAME_FINISHED, std::monostate{});
        send(GameLoopAction::SEND_LEADERBOARD, leaderboard);
        wait();
        send(GameLoopAction::RETURN_TO_LOBBY, std::monostate{});
        // The game tells the server it left when it is destroyed
        received.push_back(server.receive_game_command().client_action);
    });

    bool server_connected = false;
    {
        Game game(client, 7);
        server_connected = game.run();
    }
    server_thread.join();

    EXPECT_TRUE(server_connected);
    EXPECT_EQ(received, (std::vector<ClientAction>{ClientAction::SEND_CAR_MOVEMENT,
                                                   ClientAction::DISCONNECT}));
}

TEST_F(ConnectedSdlTest, ServerDisconnectionEndsTheGame) {
    std::thread server_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        server.send_disconnection();
        server.receive_game_command();  // the disconnection of the destroyed game
    });

    bool server_connected = true;
    {
        Game game(client, 7);
        server_connected = game.run();
    }
    server_thread.join();

    EXPECT_FALSE(server_connected);
}


// Power-ups on the track and effect badges in the HUD (render and sound milestone)

namespace {

// The draw lists are checked always; only rendering the frame in a real city, offscreen, needs
// the installed assets, so just that part is skipped without them
class PowerUpRenderTest: public ::testing::Test {
protected:
    void render(LocalInfo& state) {
        if (!std::filesystem::exists(ASSETS_FOLDER))
            GTEST_SKIP() << "the client assets are not installed in " << ASSETS_FOLDER;
        setenv("SDL_VIDEODRIVER", "offscreen", 1);
        setenv("SDL_AUDIODRIVER", "dummy", 1);

        SDLManager sdl;
        sdl.load_city("Liberty City", bridges("liberty_city"), decorations("liberty_city"));
        for (int frame = 0; frame < 3; ++frame) sdl.render(state);
    }
};

}  // namespace


TEST_F(PowerUpRenderTest, DrawsTheThreePowerUps) {
    LocalInfo state = power_up_state();

    std::vector<PowerUpSprite> sprites = power_ups_to_draw(state);

    ASSERT_EQ(sprites.size(), 3u);
    EXPECT_EQ(sprites[0].id, 1);
    EXPECT_EQ(sprites[0].sprite, NITRO_SPRITE_PATH);
    EXPECT_EQ(sprites[1].id, 2);
    EXPECT_EQ(sprites[1].sprite, LIFE_SPRITE_PATH);
    EXPECT_EQ(sprites[2].id, 3);
    EXPECT_EQ(sprites[2].sprite, SHIELD_SPRITE_PATH);
    for (const auto& sprite: sprites) EXPECT_EQ(sprite.size, CAR_RENDER_SIZE);
    EXPECT_NO_THROW(render(state));
}

TEST_F(PowerUpRenderTest, InactivePowerUpIsNotDrawn) {
    LocalInfo state = power_up_state();
    state.power_ups[1].active = false;

    EXPECT_EQ(drawn_ids(state), (std::vector<uint8_t>{1, 3}));
    EXPECT_NO_THROW(render(state));
}

TEST_F(PowerUpRenderTest, UnknownTypeIsIgnored) {
    LocalInfo state = power_up_state();
    state.power_ups.emplace_back(4, static_cast<PowerUpType>(99), 150, 100, true);

    EXPECT_EQ(drawn_ids(state), (std::vector<uint8_t>{1, 2, 3}));
    EXPECT_NO_THROW(render(state));
}

TEST(PowerUpHudTest, NitroBadgeShowsCeiledSeconds) {
    LocalInfo state = power_up_state();
    state.effects = {PowerUpEffectDTO(1, PowerUpType::Nitro, 2400)};

    std::vector<EffectBadge> badges = effect_badges(state);

    ASSERT_EQ(badges.size(), 1u);
    EXPECT_EQ(badges[0].type, PowerUpType::Nitro);
    EXPECT_EQ(badges[0].sprite, NITRO_SPRITE_PATH);
    EXPECT_EQ(badges[0].text, "3s");
}

TEST(PowerUpHudTest, TwoBadgesSideBySide) {
    LocalInfo state = power_up_state();
    state.effects = {PowerUpEffectDTO(1, PowerUpType::Nitro, 1000),
                     PowerUpEffectDTO(1, PowerUpType::Shield, 4000)};

    std::vector<EffectBadge> badges = effect_badges(state);

    ASSERT_EQ(badges.size(), 2u);
    EXPECT_EQ(badges[0].sprite, NITRO_SPRITE_PATH);
    EXPECT_EQ(badges[0].text, "1s");
    EXPECT_EQ(badges[1].sprite, SHIELD_SPRITE_PATH);
    EXPECT_EQ(badges[1].text, "4s");
    EXPECT_GT(badges[0].w, 0);
    EXPECT_GT(badges[0].h, 0);
    EXPECT_FALSE(intersect(badges[0], badges[1]));
}

TEST(PowerUpAssetsTest, SpritesAreInstalled) {
    if (!std::filesystem::exists(ASSETS_FOLDER))
        GTEST_SKIP() << "the client assets are not installed in " << ASSETS_FOLDER;

    EXPECT_TRUE(std::filesystem::exists("/var/nfs/assets/sprites/nitro.png"));
    EXPECT_TRUE(std::filesystem::exists("/var/nfs/assets/sprites/life.png"));
    EXPECT_TRUE(std::filesystem::exists("/var/nfs/assets/sprites/shield.png"));
}
