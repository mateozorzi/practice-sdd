#ifndef GAMELOBBY_H
#define GAMELOBBY_H

#include <QMainWindow>

namespace Ui {
class GameLobby;
}

class GameLobby: public QMainWindow {
    Q_OBJECT

public:
    explicit GameLobby(QWidget* parent = nullptr);
    ~GameLobby();

private:
    Ui::GameLobby* ui;
};

#endif  // GAMELOBBY_H
