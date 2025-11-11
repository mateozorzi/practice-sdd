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
    void join_lobby(const uint32_t id);
    void refresh();
    void close_menu();

public:
    JoinLobby(LobbyCatalogueDTO lobbies, QWidget* parent = nullptr);
    ~JoinLobby();

    void fill_table(const LobbyCatalogueDTO& lobbies);

private slots:
    void on_exit_clicked();

    void on_refresh_clicked();

    void on_pushButton_clicked();

    void on_join_lobby(const uint32_t id);

private:
    Ui::JoinLobby* ui;
};

#endif  // JOINLOBBY_H
