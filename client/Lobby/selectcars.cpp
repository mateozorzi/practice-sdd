#include "selectcars.h"

#include "ui_selectcars.h"
#include "../../common/car_dto.h"
#include "../../common/car_catalogue_dto.h"
#include <QLabel>

SelectCars::SelectCars(CarCatalogueDTO& catalogue, QWidget* parent):
        QMainWindow(parent)
        , ui(new Ui::SelectCars)
        , catalogue(catalogue)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);

    value_format();

    ui->Carinfo->setRowCount(catalogue.size);
    ui->Carinfo->setColumnCount(7);
    QStringList colLabels;
    colLabels <<""<< "Name" << "Health" << "Handling" << "Acceleration" << "Speed"<<"Action";;
    ui->Carinfo->setHorizontalHeaderLabels(colLabels);
    ui->Carinfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->Carinfo->horizontalHeader()->setVisible(true);

    ui->Carinfo->setColumnWidth(0, 80);

    for (int col = 1; col < 7; ++col) {
        ui->Carinfo->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }

    set_market();

}

SelectCars::~SelectCars() {
    delete ui;
    delete scene;
}

void SelectCars::on_Confirm_clicked()
{
    //QString select_car = ui->combobox->currentText();
    //emit confirm_car(select_car);
    //this->close();
}


void SelectCars::on_Exit_clicked()
{
    emit back_to_lobby();
    this->close();
}

void SelectCars::value_format(){
    QString style = R"(
        QTableWidget#Carinfo {
            background-color: transparent;
            alternate-background-color: transparent;
            text-align: center;
        }

        QTableWidget#Carinfo::item {
            color: black;
            font-size: 24pt;
            text-align: center;
            border: 1px solid black;
            padding: 8px;
            background-color: white;
        }

        QTableWidget#Carinfo::item:selected {
            background-color: #e0e0e0;
            color: black;
        }

        QHeaderView::section {
            color: white;
            font-size: 25pt;
            font-weight: bold;
            background-color: black;
            border: 1px solid black;
            text-align: center;
            padding: 8px;
        }
    )";

    ui->Carinfo->setStyleSheet(style);
}

//void SelectCars::onCarChosen(const QString& model){
//
//}

void SelectCars::set_market(){
    int row = 0;
    for (const car_dto& car: catalogue.cars){
        QString path = ":/cars/images/" + QString::fromStdString(car.model) + ".png";

        //image
        QLabel* imageLabel = new QLabel();
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        imageLabel->setPixmap(pixmap);
        imageLabel->setAlignment(Qt::AlignCenter);
        ui->Carinfo->setCellWidget(row, 0, imageLabel);

        //info
        ui->Carinfo->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(car.model)));
        ui->Carinfo->setItem(row, 2, new QTableWidgetItem(QString::number(car.health)));
        ui->Carinfo->setItem(row, 3, new QTableWidgetItem(QString::number(car.handling)));
        ui->Carinfo->setItem(row, 4, new QTableWidgetItem(QString::number(car.acceleration)));
        ui->Carinfo->setItem(row, 5, new QTableWidgetItem(QString::number(car.speed)));
        //action
        QPushButton* chooseButton = new QPushButton("Choose");
        chooseButton->setStyleSheet("background-color: green; color: white; font-weight: bold;min-height: 12px;");

                // Conectamos el botón al slot con el modelo correspondiente
        //connect(chooseButton, &QPushButton::clicked, this, [this, car]() {
        //    this->onCarChosen(QString::fromStdString(car.model));
        //});

        ui->Carinfo->setCellWidget(row, 6, chooseButton);

        row++;
    }

}

