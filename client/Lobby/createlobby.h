#ifndef CREATELOBBY_H
#define CREATELOBBY_H


#include <QMainWindow>
#include <string>

namespace Ui {
class CreateLobby;
}

class CreateLobby: public QMainWindow {
    Q_OBJECT

public:
    explicit CreateLobby(QWidget* parent = nullptr);
    ~CreateLobby();

    QString get_lobby_name();
    int get_number_players();
    void create_denied();
    void create_denied_players(uint8_t players, const std::string& message);

signals:
    void create_lobby();
    void close_menu();

private slots:
    void on_create_clicked();

    void on_exit_clicked();

private:
    Ui::CreateLobby* ui;
};

#endif  // CREATELOBBY_H
