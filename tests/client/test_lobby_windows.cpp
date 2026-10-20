#include <QLCDNumber>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTest>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "../../client/Lobby/connectioninfo.h"
#include "../../client/Lobby/creategame.h"
#include "../../client/Lobby/createlobby.h"
#include "../../client/Lobby/joinlobby.h"
#include "../../client/Lobby/lobby.h"
#include "../../client/Lobby/raceselector.h"
#include "../../client/Lobby/selectcars.h"
#include "../../client/Lobby/waitingroom.h"

#include "qt_test_utils.h"

// Unit tests of the lobby windows: the buttons are clicked with QTest and the tests check the
// signals emitted and the warnings shown.

namespace {

template <typename T>
T* child(QWidget& window, const char* name) {
    T* widget = window.findChild<T*>(name);
    EXPECT_NE(widget, nullptr) << "missing widget " << name;
    return widget;
}

void click(QWidget& window, const char* button) {
    QTest::mouseClick(child<QPushButton>(window, button), Qt::LeftButton);
}

void click_cell_button(QTableWidget* table, int row, int column) {
    auto* button = qobject_cast<QPushButton*>(table->cellWidget(row, column));
    ASSERT_NE(button, nullptr);
    button->click();
}

RaceCatalogueDTO three_races() {
    return RaceCatalogueDTO(
            {RaceDTO("park", "Liberty City", {CheckpointDTO(1), CheckpointDTO(2)}, 2),
             RaceDTO("docks", "Vice City", {CheckpointDTO(1)}, 3),
             RaceDTO("bridges", "San Andreas", {}, 1)});
}

CarCatalogueDTO two_cars() {
    return CarCatalogueDTO({CarDTO("Compact", 30, 100, 17, 4), CarDTO("Truck", 300, 50, 12, 2)});
}

class LobbyWindowsTest: public QtTest {
protected:
    ModalCloser modals;
};

}  // namespace


TEST_F(LobbyWindowsTest, UsernameWindowEmitsTheUsername) {
    Lobby window;
    QString username;
    QObject::connect(&window, &Lobby::input_username,
                     [&](const QString& name) { username = name; });

    QTest::keyClicks(child<QLineEdit>(window, "input_username"), "mateo");
    click(window, "Confirm");

    EXPECT_EQ(username, "mateo");
}

TEST_F(LobbyWindowsTest, UsernameIsRequired) {
    Lobby window;
    bool emitted = false;
    QObject::connect(&window, &Lobby::input_username, [&]() { emitted = true; });

    click(window, "Confirm");

    EXPECT_FALSE(emitted);
    EXPECT_EQ(modals.messages, QStringList{"Enter a username."});
}

TEST_F(LobbyWindowsTest, UsernameWindowExit) {
    Lobby window;
    window.show();
    bool closed = false;
    QObject::connect(&window, &Lobby::game_close, [&]() { closed = true; });

    click(window, "Exit");

    EXPECT_TRUE(closed);
    EXPECT_FALSE(window.isVisible());
}

TEST_F(LobbyWindowsTest, ConnectionWindowEmitsIpAndPort) {
    ConnectionInfo window;
    QString ip;
    QString port;
    QObject::connect(&window, &ConnectionInfo::emit_confirm_connection,
                     [&](const QString& i, const QString& p) {
                         ip = i;
                         port = p;
                     });
    bool closed = false;
    QObject::connect(&window, &ConnectionInfo::emit_close, [&]() { closed = true; });

    QTest::keyClicks(child<QLineEdit>(window, "input_ip"), "127.0.0.1");
    QTest::keyClicks(child<QLineEdit>(window, "input_port"), "8080");
    click(window, "Confirm");
    click(window, "Exit");

    EXPECT_EQ(ip, "127.0.0.1");
    EXPECT_EQ(port, "8080");
    EXPECT_TRUE(closed);
}

TEST_F(LobbyWindowsTest, CreateOrJoinWindowEmitsTheChoice) {
    creategame window;
    std::vector<std::string> signals_received;
    QObject::connect(&window, &creategame::create_lobby,
                     [&]() { signals_received.push_back("create"); });
    QObject::connect(&window, &creategame::join_lobby,
                     [&]() { signals_received.push_back("join"); });
    QObject::connect(&window, &creategame::close_menu,
                     [&]() { signals_received.push_back("close"); });

    click(window, "createlobby");
    click(window, "joinlobby");
    click(window, "Exit");

    EXPECT_EQ(signals_received, (std::vector<std::string>{"create", "join", "close"}));
}

TEST_F(LobbyWindowsTest, CreateLobbyWindowReadsNameAndPlayers) {
    CreateLobby window;
    bool created = false;
    QObject::connect(&window, &CreateLobby::create_lobby, [&]() { created = true; });

    QTest::keyClicks(child<QLineEdit>(window, "lobbyname"), "my lobby");
    child<QSpinBox>(window, "numberplayers")->setValue(4);
    click(window, "create");

    EXPECT_TRUE(created);
    EXPECT_EQ(window.get_lobby_name(), "my lobby");
    EXPECT_EQ(window.get_number_players(), 4);
}

TEST_F(LobbyWindowsTest, LobbyNameIsRequired) {
    CreateLobby window;
    bool created = false;
    QObject::connect(&window, &CreateLobby::create_lobby, [&]() { created = true; });

    click(window, "create");

    EXPECT_FALSE(created);
    EXPECT_EQ(modals.messages, QStringList{"The lobby must have a name."});
}

TEST_F(LobbyWindowsTest, CreateLobbyWindowShowsTheServerErrors) {
    CreateLobby window;
    bool closed = false;
    QObject::connect(&window, &CreateLobby::close_menu, [&]() { closed = true; });

    window.create_denied();
    window.create_denied_players(8, "Max players reached");
    click(window, "exit");

    EXPECT_EQ(modals.messages, (QStringList{"Denied create lobby.", "Max players reached: 8"}));
    EXPECT_TRUE(closed);
}

TEST_F(LobbyWindowsTest, WaitingRoomShowsTheLobby) {
    LobbyDTO lobby(LobbyAction::INFO_LOBBY, 3, "my lobby", 6);
    WaitingRoom owner_room(true, lobby);
    WaitingRoom guest_room(false, lobby);

    EXPECT_EQ(child<QLCDNumber>(owner_room, "maxPlayers")->intValue(), 6);
    EXPECT_TRUE(child<QPushButton>(owner_room, "start")->isEnabled());
    EXPECT_FALSE(child<QPushButton>(guest_room, "start")->isEnabled());

    bool started = false;
    bool closed = false;
    QObject::connect(&owner_room, &WaitingRoom::start_lobby, [&]() { started = true; });
    QObject::connect(&owner_room, &WaitingRoom::close_menu, [&]() { closed = true; });
    click(owner_room, "exit");
    click(owner_room, "start");

    EXPECT_TRUE(started);
    EXPECT_TRUE(closed);
}

TEST_F(LobbyWindowsTest, JoinWindowListsTheGames) {
    LobbyCatalogueDTO games({LobbyDTO(LobbyAction::INFO_LOBBY, 3, "first", 4, 1),
                             LobbyDTO(LobbyAction::INFO_LOBBY, 9, "second", 2, 2)});
    JoinLobby window(games);

    QTableWidget* table = child<QTableWidget>(window, "lobbytable");
    ASSERT_EQ(table->rowCount(), 2);
    EXPECT_EQ(table->item(0, 0)->text(), "3");
    EXPECT_EQ(table->item(0, 1)->text(), "first");
    EXPECT_EQ(table->item(0, 2)->text(), "1");
    EXPECT_EQ(table->item(1, 3)->text(), "2");

    window.fill_table(LobbyCatalogueDTO());
    EXPECT_EQ(table->rowCount(), 0);
}

TEST_F(LobbyWindowsTest, JoinWindowEmitsTheChosenGame) {
    LobbyCatalogueDTO games({LobbyDTO(LobbyAction::INFO_LOBBY, 3, "first", 4, 1),
                             LobbyDTO(LobbyAction::INFO_LOBBY, 9, "second", 2, 1)});
    JoinLobby window(games);
    uint32_t joined = 0;
    int refreshes = 0;
    bool closed = false;
    QObject::connect(&window, &JoinLobby::join_lobby,
                     [&](const LobbyDTO& lobby) { joined = lobby.lobby_id; });
    QObject::connect(&window, &JoinLobby::refresh, [&]() { refreshes++; });
    QObject::connect(&window, &JoinLobby::close_menu, [&]() { closed = true; });

    click_cell_button(child<QTableWidget>(window, "lobbytable"), 1, 4);
    click(window, "refresh");
    click(window, "exit");
    window.join_denied();

    EXPECT_EQ(joined, 9u);
    EXPECT_EQ(refreshes, 1);
    EXPECT_TRUE(closed);
    EXPECT_EQ(modals.messages, QStringList{"Denied join to lobby, refresh and try again."});
}

TEST_F(LobbyWindowsTest, RaceSelectorMovesRacesBetweenTables) {
    RaceCatalogueDTO races = three_races();
    RaceSelector window(races);
    QTableWidget* available = child<QTableWidget>(window, "availableRaces");
    QTableWidget* selected = child<QTableWidget>(window, "select");

    ASSERT_EQ(available->rowCount(), 3);
    EXPECT_EQ(available->item(0, 0)->text(), "park");
    EXPECT_EQ(available->item(0, 1)->text(), "Liberty City");
    EXPECT_EQ(available->item(0, 2)->text(), "2");
    EXPECT_EQ(available->item(0, 3)->text(), "2");

    click_cell_button(available, 1, 4);  // add "docks"
    ASSERT_EQ(available->rowCount(), 2);
    ASSERT_EQ(selected->rowCount(), 1);
    EXPECT_EQ(selected->item(0, 0)->text(), "docks");

    click_cell_button(selected, 0, 4);  // and remove it again
    EXPECT_EQ(available->rowCount(), 3);
    EXPECT_EQ(selected->rowCount(), 0);
    EXPECT_EQ(available->item(2, 0)->text(), "docks");
}

TEST_F(LobbyWindowsTest, RaceSelectorConfirmsTheSelectedRacesInOrder) {
    RaceCatalogueDTO races = three_races();
    RaceSelector window(races);
    std::vector<std::string> confirmed;
    QObject::connect(&window, &RaceSelector::confirm_races,
                     [&](const std::vector<RaceDTO>& chosen) {
                         std::transform(chosen.begin(), chosen.end(), std::back_inserter(confirmed),
                                        [](const RaceDTO& race) { return race.name; });
                     });

    QTableWidget* available = child<QTableWidget>(window, "availableRaces");
    click_cell_button(available, 2, 4);  // bridges
    click_cell_button(available, 0, 4);  // park
    click(window, "pushButton");

    EXPECT_EQ(confirmed, (std::vector<std::string>{"bridges", "park"}));
}

TEST_F(LobbyWindowsTest, RaceSelectorNeedsAtLeastOneRace) {
    RaceCatalogueDTO races = three_races();
    RaceSelector window(races);
    bool confirmed = false;
    bool closed = false;
    QObject::connect(&window, &RaceSelector::confirm_races, [&]() { confirmed = true; });
    QObject::connect(&window, &RaceSelector::close_menu, [&]() { closed = true; });

    click(window, "pushButton");
    click(window, "pushButton_2");

    EXPECT_FALSE(confirmed);
    EXPECT_TRUE(closed);
    EXPECT_EQ(modals.messages, QStringList{"There must be at least one race selected."});
}

TEST_F(LobbyWindowsTest, CarSelectionShowsTheCatalogue) {
    CarCatalogueDTO cars = two_cars();
    SelectCars window(cars);

    QListWidget* list = child<QListWidget>(window, "img_cars");
    ASSERT_EQ(list->count(), 2);
    EXPECT_EQ(list->item(1)->text(), "Truck");

    // The first car is chosen by default, its stats are relative to the best car
    QTableWidget* info = child<QTableWidget>(window, "Carinfo");
    EXPECT_EQ(info->item(0, 0)->text(), "Compact");
    auto* health = info->cellWidget(1, 0)->findChild<QProgressBar*>();
    auto* handling = info->cellWidget(2, 0)->findChild<QProgressBar*>();
    ASSERT_NE(health, nullptr);
    EXPECT_EQ(health->value(), 10);     // 30 of 300
    EXPECT_EQ(handling->value(), 100);  // the best handling
}

TEST_F(LobbyWindowsTest, CarSelectionConfirmsTheClickedCar) {
    CarCatalogueDTO cars = two_cars();
    SelectCars window(cars);
    std::string chosen;
    bool back = false;
    QObject::connect(&window, &SelectCars::confirm_car,
                     [&](const CarDTO& car) { chosen = car.model; });
    QObject::connect(&window, &SelectCars::back_to_lobby, [&]() { back = true; });

    click(window, "Select");
    EXPECT_EQ(chosen, "Compact");

    QListWidget* list = child<QListWidget>(window, "img_cars");
    emit list->itemClicked(list->item(1));
    EXPECT_EQ(child<QTableWidget>(window, "Carinfo")->item(0, 0)->text(), "Truck");
    click(window, "Select");
    EXPECT_EQ(chosen, "Truck");

    click(window, "Exit");
    EXPECT_TRUE(back);
}
