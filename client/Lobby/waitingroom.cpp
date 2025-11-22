#include "waitingroom.h"

#include "ui_waitingroom.h"

WaitingRoom::WaitingRoom(const bool is_owner, const LobbyDTO& lobby, QWidget* parent):
        QMainWindow(parent), ui(new Ui::WaitingRoom), is_owner(is_owner), lobby(lobby) {
    ui->setupUi(this);

    ui->start->setCheckable(is_owner);
    ui->start->setEnabled(is_owner);
    ui->start->setVisible(is_owner);

    ui->maxPlayers->display(lobby.max_players);
    ui->lobbyname->setText(QString::fromStdString(lobby.lobby_name));
}

WaitingRoom::~WaitingRoom() { delete ui; }

void WaitingRoom::on_start_clicked() {
    emit start_lobby();
    this->close();
}


void WaitingRoom::on_exit_clicked() { emit close_menu(); }
