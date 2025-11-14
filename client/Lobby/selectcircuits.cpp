#include "selectcircuits.h"

#include "ui_selectcircuits.h"

SelectCircuits::SelectCircuits(QWidget* parent): QMainWindow(parent), ui(new Ui::SelectCircuits) {
    ui->setupUi(this);
}

SelectCircuits::~SelectCircuits() { delete ui; }
