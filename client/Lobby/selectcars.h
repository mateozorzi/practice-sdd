#ifndef SELECTCARS_H
#define SELECTCARS_H

#include <QMainWindow>
#include <QGraphicsScene>
#include "../../common/car_dto.h"
#include "../../common/car_catalogue_dto.h"

namespace Ui {
class SelectCars;
}

class SelectCars: public QMainWindow {
    Q_OBJECT

signals:
    void back_to_lobby();
    void confirm_car(const car_dto& car);

public:
    explicit SelectCars(CarCatalogueDTO& catalogue, QWidget* parent = nullptr);
    ~SelectCars();

private slots:
    void value_format();

    void on_Exit_clicked();



    void on_Select_clicked();

private:
    Ui::SelectCars* ui;
    QGraphicsScene* scene;
    CarCatalogueDTO catalogue;
    car_dto actual_car;

    void onCarChosen(const car_dto& car);

    void set_market();

    QString getProgressBarStyle(const int value);
    QWidget* createProgressBar(uint8_t value);
    car_dto inspect_catalogue();
    car_dto normalize_value_car(const car_dto& car, const car_dto& normalized);
};

#endif  // SELECTCARS_H
