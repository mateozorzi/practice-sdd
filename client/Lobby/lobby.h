#ifndef LOBBY_H
#define LOBBY_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class Lobby;
}
QT_END_NAMESPACE

class Lobby: public QMainWindow {
    Q_OBJECT

public:
    explicit Lobby(QWidget* parent = nullptr);
    ~Lobby();

signals:
    void input_username(const QString& username);
    void game_close();

private slots:

    void on_Confirm_clicked();

    void on_Exit_clicked();

private:
    Ui::Lobby* ui;
    QString username;
};
#endif  // LOBBY_H
