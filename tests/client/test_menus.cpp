#include <QAudioOutput>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "../../client/Lobby/connectioninfo.h"
#include "../../client/Lobby/creategame.h"
#include "../../client/Lobby/createlobby.h"
#include "../../client/Lobby/joinlobby.h"
#include "../../client/Lobby/lobby.h"
#include "../../client/Lobby/raceselector.h"
#include "../../client/Lobby/selectcars.h"
#include "../../client/client_protocol.h"
#include "../../client/menu_connection.h"
#include "../../client/menu_interface.h"
#include "../../common/socket.h"
#include "../../server/server_protocol.h"

#include "qt_test_utils.h"

// MenuConnection runs its own window until the user confirms or exits. MenuInterface is tested
// end to end: the real windows and client protocol against a scripted server on a socket, with
// a robot clicking through the windows as a user would.

namespace {

std::atomic<int> next_port{28500};

int menu_argc = 1;
char menu_name[] = "nfs_client";
char* menu_argv[] = {menu_name, nullptr};

void click(QWidget& window, const char* button) { window.findChild<QPushButton*>(button)->click(); }

void type(QWidget& window, const char* line_edit, const QString& text) {
    window.findChild<QLineEdit*>(line_edit)->setText(text);
}

void click_cell_button(QWidget& window, const char* table, int row, int column) {
    auto* cell = window.findChild<QTableWidget*>(table)->cellWidget(row, column);
    qobject_cast<QPushButton*>(cell)->click();
}

// Waits for a visible window of type T and acts on it from a fresh event (the menus block
// inside nested event loops, so the action must not run inside the robot's own timer)
template <typename T>
std::function<bool()> when(std::function<void(T&)> action) {
    return [action]() {
        if (QApplication::activeModalWidget())
            return false;
        T* window = visible_window<T>();
        if (!window)
            return false;
        QTimer::singleShot(0, window, [window, action]() { action(*window); });
        return true;
    };
}

// Runs the steps in order, each one once its window shows up
class Script {
    std::vector<std::function<bool()>> steps;
    size_t next = 0;
    UiRobot robot;

public:
    explicit Script(std::vector<std::function<bool()>> steps):
            steps(std::move(steps)), robot([this]() {
                if (next < this->steps.size() && this->steps[next]())
                    next++;
                return next >= this->steps.size();
            }) {}

    bool finished() const { return next >= steps.size(); }
};

std::pair<std::string, std::string> run_connection_menu(const QString& ip, const QString& port,
                                                        bool exit = false) {
    on_next_application([ip, port, exit]() {
        ConnectionInfo* window = visible_window<ConnectionInfo>();
        ASSERT_NE(window, nullptr);
        if (exit) {
            click(*window, "Exit");
            return;
        }
        type(*window, "input_ip", ip);
        type(*window, "input_port", port);
        click(*window, "Confirm");
    });
    return MenuConnection().run();
}

RaceCatalogueDTO server_races() {
    return RaceCatalogueDTO({RaceDTO("park", "Liberty City", {CheckpointDTO(1, 'H', 10, 10)}, 2),
                             RaceDTO("docks", "Vice City", {CheckpointDTO(1, 'V', 20, 20)}, 1)});
}

CarCatalogueDTO server_cars() {
    return CarCatalogueDTO({CarDTO("Compact", 30, 100, 17, 4), CarDTO("Jeep", 120, 80, 20, 5)});
}

// Client connected to a scripted server running in its own thread
class MenuInterfaceTest: public ::testing::Test {
protected:
    std::string port = std::to_string(next_port++);
    Socket acceptor{port.c_str()};
    Socket client_socket{"localhost", port.c_str()};
    Socket server_socket = acceptor.accept();
    ClientProtocol client{client_socket};
    ServerProtocol server{server_socket};
    std::thread server_thread;
    PlayerDTO player;

    void SetUp() override { use_offscreen_platform(); }

    void serve(std::function<void(ServerProtocol&)> script) {
        server_thread = std::thread([this, script]() {
            try {
                script(server);
            } catch (const std::exception& e) {
                ADD_FAILURE() << "server script failed: " << e.what();
            }
        });
    }

    void TearDown() override {
        if (server_thread.joinable())
            server_thread.join();
    }

    // Answers the username and returns the request
    static LobbyDTO accept_username(ServerProtocol& server, uint32_t id) {
        LobbyDTO request = server.receive_lobby_info();
        server.send_player_id(id);
        return request;
    }
};

}  // namespace


TEST(MenuConnectionTest, ReturnsAValidAddress) {
    use_offscreen_platform();

    testing::internal::CaptureStdout();
    auto [ip, port] = run_connection_menu("127.0.0.1", "8080");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(ip, "127.0.0.1");
    EXPECT_EQ(port, "8080");
    EXPECT_NE(output.find("Connecting to IP: 127.0.0.1 Port: 8080"), std::string::npos);
}

TEST(MenuConnectionTest, RejectsInvalidAddresses) {
    use_offscreen_platform();
    const std::pair<std::string, std::string> nothing("", "");

    testing::internal::CaptureStdout();
    EXPECT_EQ(run_connection_menu("", "8080"), nothing);
    EXPECT_EQ(run_connection_menu("127.0.0.1", ""), nothing);
    EXPECT_EQ(run_connection_menu("127.0.0.1", "port"), nothing);
    EXPECT_EQ(run_connection_menu("127.0.0.1", "70000"), nothing);
    EXPECT_EQ(run_connection_menu("localhost", "8080"), nothing);
    EXPECT_EQ(run_connection_menu("127.0.0.300", "8080"), nothing);
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Invalid port number"), std::string::npos);
    EXPECT_NE(output.find("Invalid IP address"), std::string::npos);
}

TEST(MenuConnectionTest, ExitReturnsNothing) {
    use_offscreen_platform();

    EXPECT_EQ(run_connection_menu("", "", true), std::make_pair(std::string(), std::string()));
}


TEST_F(MenuInterfaceTest, CreatesAGameAndSelectsACar) {
    LobbyDTO username_request;
    LobbyDTO races_selection;
    LobbyDTO create_request;
    CarDTO selected_car;
    serve([&](ServerProtocol& server) {
        username_request = accept_username(server, 7);
        EXPECT_EQ(server.receive_lobby_info().action, LobbyAction::LIST_RACES);
        server.send_races_catalogue(server_races());
        races_selection = server.receive_lobby_info();
        create_request = server.receive_lobby_info();
        server.send_lobby_response(LobbyDTO(LobbyAction::CREATE_OR_JOIN_SUCCESS));
        server.send_car_catalogue(server_cars());
        GameCommandDTO car = server.receive_game_command();
        selected_car = car.get_data<CarDTO>()->get();
    });

    MenuInterface menu(client, menu_argc, menu_argv);
    Script script({when<Lobby>([](Lobby& w) {
                       type(w, "input_username", "mateo");
                       click(w, "Confirm");
                   }),
                   when<creategame>([](creategame& w) { click(w, "createlobby"); }),
                   when<CreateLobby>([](CreateLobby& w) {
                       type(w, "lobbyname", "my game");
                       w.findChild<QSpinBox*>("numberplayers")->setValue(3);
                       click(w, "create");
                   }),
                   when<RaceSelector>([](RaceSelector& w) {
                       click_cell_button(w, "availableRaces", 1, 4);
                       click(w, "pushButton");
                   }),
                   when<SelectCars>([](SelectCars& w) { click(w, "Select"); })});
    menu.lobby(player);

    EXPECT_TRUE(script.finished());
    EXPECT_EQ(player.id, 7u);
    EXPECT_EQ(player.username, "mateo");
    EXPECT_EQ(player.car.model, "Compact");

    server_thread.join();
    EXPECT_EQ(username_request.action, LobbyAction::CHANGE_USERNAME);
    EXPECT_EQ(username_request.lobby_name, "mateo");
    EXPECT_EQ(races_selection.action, LobbyAction::SEND_RACE_SELECTION);
    ASSERT_EQ(races_selection.selected_races.races.size(), 1u);
    EXPECT_EQ(races_selection.selected_races.races[0].name, "docks");
    EXPECT_EQ(create_request.action, LobbyAction::CREATE_LOBBY);
    EXPECT_EQ(create_request.lobby_name, "my game");
    EXPECT_EQ(create_request.max_players, 3);
    EXPECT_EQ(selected_car.model, "Compact");
}

TEST_F(MenuInterfaceTest, ShowsTheServerErrorsWhenCreatingAGame) {
    serve([&](ServerProtocol& server) {
        accept_username(server, 7);
        for (LobbyAction answer:
             {LobbyAction::MAX_PLAYERS_ERROR, LobbyAction::MIN_PLAYERS_ERROR,
              LobbyAction::CREATE_OR_JOIN_ERROR, LobbyAction::CREATE_OR_JOIN_SUCCESS}) {
            server.receive_lobby_info();  // races request
            server.send_races_catalogue(server_races());
            server.receive_lobby_info();  // races selection
            server.receive_lobby_info();  // create request
            server.send_lobby_response(LobbyDTO(answer, 0, "", 8));
        }
        server.send_car_catalogue(server_cars());
        server.receive_game_command();
    });

    MenuInterface menu(client, menu_argc, menu_argv);
    ModalCloser modals;
    auto create = when<CreateLobby>([](CreateLobby& w) {
        type(w, "lobbyname", "my game");
        click(w, "create");
    });
    auto choose_race = when<RaceSelector>([](RaceSelector& w) {
        click_cell_button(w, "availableRaces", 0, 4);
        click(w, "pushButton");
    });
    Script script({when<Lobby>([](Lobby& w) {
                       type(w, "input_username", "mateo");
                       click(w, "Confirm");
                   }),
                   when<creategame>([](creategame& w) { click(w, "createlobby"); }), create,
                   choose_race, create, choose_race, create, choose_race, create, choose_race,
                   when<SelectCars>([](SelectCars& w) { click(w, "Select"); })});
    menu.lobby(player);

    EXPECT_TRUE(script.finished());
    EXPECT_EQ(modals.messages, (QStringList{"Max players reached: 8", "Min players required: 8",
                                            "Denied create lobby."}));
}

TEST_F(MenuInterfaceTest, JoinsAGameAfterADenial) {
    LobbyCatalogueDTO games({LobbyDTO(LobbyAction::INFO_LOBBY, 3, "first", 4, 1),
                             LobbyDTO(LobbyAction::INFO_LOBBY, 9, "second", 4, 2)});
    std::vector<uint32_t> join_requests;
    serve([&](ServerProtocol& server) {
        accept_username(server, 7);
        EXPECT_EQ(server.receive_lobby_info().action, LobbyAction::SEND_LOBBY_LIST);
        server.send_active_games(games);
        EXPECT_EQ(server.receive_lobby_info().action, LobbyAction::SEND_LOBBY_LIST);  // refresh
        server.send_active_games(games);
        join_requests.push_back(server.receive_lobby_info().lobby_id);
        server.send_lobby_response(LobbyDTO(LobbyAction::CREATE_OR_JOIN_ERROR));
        join_requests.push_back(server.receive_lobby_info().lobby_id);
        server.send_lobby_response(LobbyDTO(LobbyAction::CREATE_OR_JOIN_SUCCESS));
        server.send_car_catalogue(server_cars());
        // The player goes back from the car selection: the game sends it to the lobby
        EXPECT_EQ(server.receive_game_command().client_action, ClientAction::DISCONNECT);
        server.send_game_update(GameUpdateDTO(GameLoopAction::RETURN_TO_LOBBY));
        // and then closes the game
        EXPECT_EQ(server.receive_lobby_info().action, LobbyAction::EXIT_GAME);
    });

    MenuInterface menu(client, menu_argc, menu_argv);
    ModalCloser modals;
    auto join_second =
            when<JoinLobby>([](JoinLobby& w) { click_cell_button(w, "lobbytable", 1, 4); });
    Script script({when<Lobby>([](Lobby& w) {
                       type(w, "input_username", "mateo");
                       click(w, "Confirm");
                   }),
                   when<creategame>([](creategame& w) { click(w, "joinlobby"); }),
                   when<JoinLobby>([](JoinLobby& w) { click(w, "refresh"); }), join_second,
                   join_second, when<SelectCars>([](SelectCars& w) { click(w, "Exit"); }),
                   when<Lobby>([](Lobby& w) { click(w, "Exit"); })});
    menu.lobby(player);

    EXPECT_TRUE(script.finished());
    EXPECT_EQ(player.id, static_cast<uint32_t>(-1));
    EXPECT_EQ(modals.messages, QStringList{"Denied join to lobby, refresh and try again."});
    server_thread.join();
    EXPECT_EQ(join_requests, (std::vector<uint32_t>{9, 9}));
}

TEST_F(MenuInterfaceTest, MenuMusicIsQuiet) {
    MenuInterface menu(client, menu_argc, menu_argv);
    QObject window;

    menu.set_music(&window);

    const auto* output = window.findChild<QAudioOutput*>();
    ASSERT_NE(output, nullptr);
    EXPECT_FLOAT_EQ(output->volume(), 0.05f);
}

TEST_F(MenuInterfaceTest, ClosingTheMenusGoesBack) {
    serve([&](ServerProtocol& server) {
        accept_username(server, 7);
        server.receive_lobby_info();  // races request
        server.send_races_catalogue(server_races());
        server.receive_lobby_info();  // lobby list
        server.send_active_games(LobbyCatalogueDTO());
        EXPECT_EQ(server.receive_lobby_info().action, LobbyAction::EXIT_GAME);
    });

    MenuInterface menu(client, menu_argc, menu_argv);
    Script script({when<Lobby>([](Lobby& w) {
                       type(w, "input_username", "mateo");
                       click(w, "Confirm");
                   }),
                   when<creategame>([](creategame& w) { click(w, "createlobby"); }),
                   when<CreateLobby>([](CreateLobby& w) {
                       type(w, "lobbyname", "my game");
                       click(w, "create");
                   }),
                   when<RaceSelector>([](RaceSelector& w) { click(w, "pushButton_2"); }),
                   when<creategame>([](creategame& w) { click(w, "joinlobby"); }),
                   when<JoinLobby>([](JoinLobby& w) { click(w, "exit"); }),
                   when<creategame>([](creategame& w) { click(w, "createlobby"); }),
                   when<CreateLobby>([](CreateLobby& w) { click(w, "exit"); }),
                   when<creategame>([](creategame& w) { click(w, "Exit"); }),
                   when<Lobby>([](Lobby& w) { click(w, "Exit"); })});
    menu.lobby(player);

    EXPECT_TRUE(script.finished());
    EXPECT_EQ(player.id, static_cast<uint32_t>(-1));
}
