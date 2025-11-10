#ifndef CREATEGAME_H
#define CREATEGAME_H

#include <QMainWindow>

namespace Ui {
class creategame;
}

class creategame: public QMainWindow {
    Q_OBJECT

public:
    explicit creategame(QWidget* parent = nullptr);
    ~creategame();

signals:
    void create_lobby();
    void join_lobby();
    void close_menu();

private slots:
    void on_createlobby_clicked();

    void on_joinlobby_clicked();

    void on_Exit_clicked();

private:
    Ui::creategame* ui;
};

#endif  // CREATEGAME_H
