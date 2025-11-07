#include "selectcars.h"

#include "ui_selectcars.h"
#include "../../common/car_dto.h"
#include "../../common/car_catalogue_dto.h"
#include <QLabel>

#define MAX_IMG 64

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
    ui->Carinfo->verticalHeader()->setVisible(false);

    ui->Carinfo->setColumnWidth(0, MAX_IMG+15);

    for (int row = 0; row < catalogue.size; ++row) {
        ui->Carinfo->setRowHeight(row, 20);
    }

    for (int col = 1; col < 7; ++col) {
        ui->Carinfo->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }

    set_market();

}

SelectCars::~SelectCars() {
    delete ui;
    delete scene;
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
    }

    QTableWidget#Carinfo::item {
        color: black;
        font-size: 14pt;
        border: 1px solid black;
        background-color: white;
        text-align: center;
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
    }

    QTableWidget#Carinfo QLabel {
        background-color: white;
        border: none;
        font-size: 14pt;
    }
)";

    ui->Carinfo->setStyleSheet(style);
}

void SelectCars::onCarChosen(const car_dto& car){
    emit confirm_car(car);
}

void SelectCars::set_market(){
    int row = 0;
    for (const car_dto& car: catalogue.cars){
        QString path = ":/cars/images/" + QString::fromStdString(car.model) + ".png";

        //image
        QLabel* imageLabel = new QLabel();
        QPixmap pixmap(path);
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(MAX_IMG, MAX_IMG, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        imageLabel->setPixmap(pixmap);
        imageLabel->setAlignment(Qt::AlignCenter);
        ui->Carinfo->setCellWidget(row, 0, imageLabel);

        //info
        QFont itemFont;
        itemFont.setPointSize(14);
        itemFont.setBold(false);
        QStringList carData = {
                QString::fromStdString(car.model),
                QString::number(car.health),
                QString::number(car.handling),
                QString::number(car.acceleration),
                QString::number(car.speed)
        };

        for (int col = 0; col < carData.size(); col++) {
            QTableWidgetItem* item = new QTableWidgetItem(carData[col]);
            item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            item->setFont(itemFont);
            item->setForeground(Qt::black);
            ui->Carinfo->setItem(row, col + 1, item);
        }
        //action


        QPushButton* chooseButton = new QPushButton("Choose");
        chooseButton->setStyleSheet("background-color: green; color: white; font-weight: bold;min-height: 35px;");

        connect(chooseButton, &QPushButton::clicked, this, [this, car]() {
            this->onCarChosen(car);
        });

        ui->Carinfo->setCellWidget(row, 6, chooseButton);

        row++;
    }

}

