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
    void confirm_car(QString select_car);

public:
    explicit SelectCars(CarCatalogueDTO& catalogue, QWidget* parent = nullptr);
    ~SelectCars();

private slots:
    void value_format();

    void on_Confirm_clicked();

    void on_Exit_clicked();

    //void onCarChosen(const QString& model);

private:
    Ui::SelectCars* ui;
    QGraphicsScene* scene;
    CarCatalogueDTO catalogue;

    void set_market();
};

#endif  // SELECTCARS_H
