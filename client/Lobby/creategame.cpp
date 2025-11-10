#include "creategame.h"

#include "ui_creategame.h"

creategame::creategame(QWidget* parent): QMainWindow(parent), ui(new Ui::creategame) {
    ui->setupUi(this);
}

creategame::~creategame() { delete ui; }

void creategame::on_createlobby_clicked()
{
    //create a lobby, the client is the anfitrion
    emit create_lobby();
}


void creategame::on_joinlobby_clicked()
{
    //the client wants to join an already create lobby
    emit join_lobby();
}


void creategame::on_Exit_clicked()
{
    emit close_menu();
    this->close();
}

