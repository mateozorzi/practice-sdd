#ifndef SELECTCARS_H
#define SELECTCARS_H

#include <QMainWindow>

namespace Ui {
class SelectCars;
}

class SelectCars: public QMainWindow {
    Q_OBJECT

signals:
    void back_to_lobby();
    void confirm_car(QString select_car);

public:
    explicit SelectCars(QWidget* parent = nullptr);
    ~SelectCars();

private slots:
    void on_Confirm_clicked();

    void on_Exit_clicked();

private:
    Ui::SelectCars* ui;
};

#endif  // SELECTCARS_H
