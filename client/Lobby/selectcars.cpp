#include "selectcars.h"

#include "ui_selectcars.h"

SelectCars::SelectCars(QWidget* parent): QMainWindow(parent), ui(new Ui::SelectCars) {
    ui->setupUi(this);
}

SelectCars::~SelectCars() { delete ui; }

void SelectCars::on_Confirm_clicked()
{
    QString select_car = ui->comboBox->currentText();
    emit confirm_car(select_car);
    this->close();
}


void SelectCars::on_Exit_clicked()
{
    emit back_to_lobby();
    this->close();
}

