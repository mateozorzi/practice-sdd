#ifndef SELECTCARS_H
#define SELECTCARS_H

#include <QGraphicsScene>
#include <QMainWindow>

#include "../../common/car_catalogue_dto.h"
#include "../../common/car_dto.h"

namespace Ui {
class SelectCars;
}

class SelectCars: public QMainWindow {
    Q_OBJECT

signals:
    void back_to_lobby();
    void confirm_car(const CarDTO& car);

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
    CarDTO actual_car;

    void onCarChosen(const CarDTO& car);

    void set_market();

    QString getProgressBarStyle(const int value);
    QWidget* createProgressBar(uint8_t value);
    CarDTO inspect_catalogue();
    CarDTO normalize_value_car(const CarDTO& car, const CarDTO& normalized);
};

#endif  // SELECTCARS_H
