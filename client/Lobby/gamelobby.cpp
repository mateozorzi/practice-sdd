#include "gamelobby.h"

#include "ui_gamelobby.h"

GameLobby::GameLobby(QWidget* parent): QMainWindow(parent), ui(new Ui::GameLobby) {
    ui->setupUi(this);
}

GameLobby::~GameLobby() { delete ui; }
