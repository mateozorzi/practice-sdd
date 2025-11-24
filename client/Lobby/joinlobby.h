#ifndef JOINLOBBY_H
#define JOINLOBBY_H

#include <QMainWindow>

#include "../../common/lobbies_catalogue_dto.h"

namespace Ui {
class JoinLobby;
}

class JoinLobby: public QMainWindow {
    Q_OBJECT

signals:
    void join_lobby(const LobbyDTO& lobby);
    void refresh();
    void close_menu();

public:
    explicit JoinLobby(LobbyCatalogueDTO lobbies, QWidget* parent = nullptr);
    ~JoinLobby();

    void fill_table(const LobbyCatalogueDTO& lobbies);
    void join_denied();

private slots:
    void on_exit_clicked();

    void on_refresh_clicked();

    void on_join_lobby(const LobbyDTO& lobby);

private:
    Ui::JoinLobby* ui;
};

#endif  // JOINLOBBY_H
