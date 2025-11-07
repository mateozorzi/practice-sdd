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

private slots:
    void on_createlobby_clicked();

    void on_joinlobby_clicked();

private:
    Ui::creategame* ui;
};

#endif  // CREATEGAME_H
