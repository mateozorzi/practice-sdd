#include "lobby.h"
#include <QMessageBox>

#include "./ui_lobby.h"

Lobby::Lobby(QWidget* parent): QMainWindow(parent), ui(new Ui::Lobby), username("") { ui->setupUi(this); }

Lobby::~Lobby() { delete ui; }


void Lobby::on_Confirm_clicked()
{
    username = ui->input_username->text();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "Error", "Enter a username.");
        return;
    }
    emit input_username(username);
}


void Lobby::on_Exit_clicked()
{
    emit game_close();
    this->close();
}

