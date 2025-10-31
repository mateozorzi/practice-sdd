#ifndef SELECTCARS_H
#define SELECTCARS_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "../../common/car_dto.h"

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

    void set_market(const std::vector<car_dto>& cars);

private slots:
    void value_format();
    void rows_format();

    void onCarSelected(int index);

    void on_Confirm_clicked();

    void on_Exit_clicked();

private:
    Ui::SelectCars* ui;
    QGraphicsScene* scene;
    std::vector<car_dto> market;
};

#endif  // SELECTCARS_H
