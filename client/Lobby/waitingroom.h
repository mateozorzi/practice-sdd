#ifndef WAITINGROOM_H
#define WAITINGROOM_H

#include <QMainWindow>

#include "../../common/lobby_dto.h"

namespace Ui {
class WaitingRoom;
}

class WaitingRoom: public QMainWindow {
    Q_OBJECT

signals:
    void close_menu();
    void start_lobby();

public:
    explicit WaitingRoom(const bool is_owner, const LobbyDTO& lobby, QWidget* parent = nullptr);
    ~WaitingRoom();

private slots:
    void on_start_clicked();

    void on_exit_clicked();

private:
    Ui::WaitingRoom* ui;
    bool is_owner;
    LobbyDTO lobby;
};

#endif  // WAITINGROOM_H
