#include "waitingroom.h"

#include "ui_waitingroom.h"

WaitingRoom::WaitingRoom(const bool is_owner, QWidget* parent): QMainWindow(parent), ui(new Ui::WaitingRoom), is_owner(is_owner) {
    ui->setupUi(this);

    ui->start->setCheckable(is_owner);
    ui->start->setEnabled(is_owner);
    ui->start->setVisible(is_owner);
}

WaitingRoom::~WaitingRoom() { delete ui; }

void WaitingRoom::on_start_clicked()
{
    emit start_lobby();
    this->close();
}


void WaitingRoom::on_exit_clicked()
{
    emit close_menu();
    this->close();
}

