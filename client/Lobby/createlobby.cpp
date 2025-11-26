#include "createlobby.h"

#include <QMessageBox>

#include "ui_createlobby.h"

CreateLobby::CreateLobby(QWidget* parent): QMainWindow(parent), ui(new Ui::CreateLobby) {
    ui->setupUi(this);
}

CreateLobby::~CreateLobby() { delete ui; }

QString CreateLobby::get_lobby_name() { return ui->lobbyname->text(); }

int CreateLobby::get_number_players() { return ui->numberplayers->value(); }

void CreateLobby::on_create_clicked() {
    // lobby name have to be complete
    QString username = ui->lobbyname->text();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "Error", "The lobby must have a name.");
        return;
    }

    // number of players <= 0
    int players = ui->numberplayers->value();
    if (players <= 0) {
        QMessageBox::warning(this, "Error", "The minimum of players is 1.");
        return;
    } else if (players > 15) {
        QMessageBox::warning(this, "Error", "The maximum of players is 15.");
        return;
    }

    emit create_lobby();
}

void CreateLobby::create_denied() {
    QMessageBox::warning(this, "Create Denied", "Denied create lobby.");
}

void CreateLobby::on_exit_clicked() {
    this->close();
    emit close_menu();
}
