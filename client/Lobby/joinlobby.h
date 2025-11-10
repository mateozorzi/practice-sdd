#ifndef JOINLOBBY_H
#define JOINLOBBY_H

#include <QMainWindow>

namespace Ui {
class JoinLobby;
}

class JoinLobby: public QMainWindow {
    Q_OBJECT

signals:
    void join_lobby();
    void refresh();
    void close_menu();

public:
    explicit JoinLobby(QWidget* parent = nullptr);
    ~JoinLobby();

private slots:
    void on_exit_clicked();

    void on_refresh_clicked();

    void on_pushButton_clicked();

private:
    Ui::JoinLobby* ui;
};

#endif  // JOINLOBBY_H
