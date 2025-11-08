#include "joinlobby.h"

#include "ui_joinlobby.h"

JoinLobby::JoinLobby(QWidget* parent): QMainWindow(parent), ui(new Ui::JoinLobby) {
    ui->setupUi(this);
}

JoinLobby::~JoinLobby() { delete ui; }

void JoinLobby::on_exit_clicked()
{
    emit close_menu();
}


void JoinLobby::on_refresh_clicked()
{
    emit refresh();
}


void JoinLobby::on_pushButton_clicked()
{
    emit join_lobby();
}

